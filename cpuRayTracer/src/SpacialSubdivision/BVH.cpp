#include "spacialSubdivision/bvh.hpp"
#include <gtx/transform.hpp>
#include <algorithm>
#include <geometric.hpp>
#include "memory.hpp"
#include "ray.hpp"

namespace CRT
{
	BVH::BVH() : 
		Primitive(),
		m_nodes(nullptr)
	{}

	void BVH::Initialize(const BVHConfig& config, const std::vector<std::shared_ptr<Primitive>>& primitives)
	{
		if (primitives.size() <= 0)
		{
			printf("WARNING: No primitives found in the scene, cannot create BVH!\n");
			return;
		}

		m_primitives = primitives;
		m_config = config;

		std::vector<BVHPrimitiveInfo> primitiveInfo(m_primitives.size());
		for(int i = 0; i < m_primitives.size(); ++i)
		{
			primitiveInfo[i] = {
				i,
				m_primitives[i]->GetWorldAabb()
			};
		}

		int totalNodes = 0;
		std::vector<std::shared_ptr<Primitive>> orderedPrimitives;
		orderedPrimitives.reserve(m_primitives.size());

		printf("Started BVH construction!\n");
		BVHBuildNode* root = Build(primitiveInfo, 0, static_cast<int>(primitiveInfo.size()), totalNodes, orderedPrimitives);
		printf("Finished BVH construction!\n");

		m_primitives.swap(orderedPrimitives);
		primitiveInfo.resize(0);

		m_nodes = AllocAligned<LinearBVHNode>(totalNodes);

		int offset = 0;
		FlattenBVHTree(root, offset);
		if(totalNodes != offset)
		{
			printf("The BVH tree failed to flatten!");
			return;
		}
	}

	bool BVH::Intersect(const Ray& ray, HitInfo& lastHitInfo, const float maxRayLength)
	{
		bool hit = false;
		glm::vec3 inverseDirection = glm::vec3(
			1.0f / ray.Direction.x,
			1.0f / ray.Direction.y,
			1.0f / ray.Direction.z
		);

		std::array<int, 3> directionIsNegative = {
			inverseDirection.x < 0,
			inverseDirection.y < 0,
			inverseDirection.z < 0
		};

		int toVisitOffset = 0;
		int currentNodeIndex = 0;
		int nodesToVisit[64];

		lastHitInfo.Distance = maxRayLength;
		HitInfo hitInfo;

		while (true)
		{
			const LinearBVHNode* node = &m_nodes[currentNodeIndex];
			if (node->Bounds.Intersect(ray, inverseDirection, directionIsNegative, maxRayLength))
			{
				if (node->NumPrimitives > 0)
				{
					for (int i = 0; i < node->NumPrimitives; ++i)
					{
						if (m_primitives[node->PrimitiveOffset + i]->Intersect(ray, hitInfo, maxRayLength) && hitInfo.Distance < lastHitInfo.Distance)
						{
							lastHitInfo = hitInfo;
						}
					}

					if (toVisitOffset == 0)
					{
						break;
					}

					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					if (directionIsNegative[node->Axis])
					{
						nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
						currentNodeIndex = node->SecondChildOffset;
					}
					else
					{
						nodesToVisit[toVisitOffset++] = node->SecondChildOffset;
						currentNodeIndex = currentNodeIndex + 1;
					}
#if defined RENDER_BVH_HEATMAP
					++lastHitInfo.TraversalSteps;
#endif
				}
			}
			else
			{
				if (toVisitOffset == 0)
				{
					break;
				}
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
		}

		return lastHitInfo.Distance < maxRayLength;
	}

	BVH::BVHBuildNode* BVH::Build(std::vector<BVH::BVHPrimitiveInfo>& primitiveInfo, int start, int end, int& totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives)
	{
		BVHBuildNode* node = new BVHBuildNode();
		int numPrimitives = end - start;
		totalNodes++;

		//Calculate the outer bounds of this node.
		AABB nodeBounds = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
		for (int i = start; i < end; i++)
		{
			nodeBounds = AABB::Combine(nodeBounds, primitiveInfo[i].Bounds);
		}

		//Check whether this node is a leaf or branch node.
		if(numPrimitives == 1)
		{
			//When only one primitive is available, create a leaf node with the given properties.
			int firstPrimitiveOffset = static_cast<int>(orderedPrimitives.size());
			for(int i = start; i < end; ++i)
			{
				orderedPrimitives.push_back(m_primitives[primitiveInfo[i].PrimitiveIndex]);
			}
			InitializeAsLeafNode(node, firstPrimitiveOffset, numPrimitives, nodeBounds);
			return node;
		}
		else
		{
			//When more than 1 primitive is found in the AABB confinement, create a branch node.
			AABB centroidBounds = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));;
			for (int i = start; i < end; ++i)
			{
				centroidBounds = AABB::Combine(centroidBounds, primitiveInfo[i].Centroid);
			}
			SortAxis sortAxis = GetSortAxis(centroidBounds, start, end);
			int mid = (start + end) / 2;

			//Check if the centroid points fall on the same position (i.e. the centroid bounds have 0 volume).
			if(centroidBounds.GetMax()[sortAxis] == centroidBounds.GetMin()[sortAxis])
			{
				int firstPrimitiveOffset = static_cast<int>(orderedPrimitives.size());
				for (int i = start; i < end; ++i)
				{
					orderedPrimitives.push_back(m_primitives[primitiveInfo[i].PrimitiveIndex]);
				}
				InitializeAsLeafNode(node, firstPrimitiveOffset, numPrimitives, nodeBounds);
				return node;
			}
			else
			{
				switch (m_config.PartitionType)
				{
				case PartitionType::Middle:
				{
					//Partition through the node's midpoint.
					float pmid = (centroidBounds.GetMin()[sortAxis] + centroidBounds.GetMax()[sortAxis]) * 0.5f;
					BVHPrimitiveInfo* midPtr = std::partition(
						&primitiveInfo[start],
						&primitiveInfo[end - 1] + 1,
						[sortAxis, pmid](const BVHPrimitiveInfo& info)
						{
							return info.Centroid[sortAxis] < pmid;
						});

					int mid = static_cast<int>(std::distance(primitiveInfo.data(), midPtr));

					//In case overlapping AABBs seem to fail, try the equal counts approach.
					if (mid != start && mid != end)
						break;
				}
				case PartitionType::EqualCounts:
				{
					//Split the node into 2 equal sized leafs.
					int mid = (start - end) / 2;
					std::nth_element(&primitiveInfo[start], &primitiveInfo[mid], &primitiveInfo[end - 1] + 1,
						[sortAxis](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b)
						{
							return a.Centroid[sortAxis] < b.Centroid[sortAxis];
						});
					break;
				}
				case PartitionType::SurfaceAreaHeuristic:
				{
					//Create the buckets for the sorting axis partition line.
					std::vector<BucketInfo> buckets(m_config.BinningCount);
					for (int i = start; i < end; ++i)
					{
						int b = static_cast<int>(buckets.size() * centroidBounds.GetOffset(primitiveInfo[i].Centroid)[sortAxis]);
						if (b == m_config.BinningCount)
						{
							b = m_config.BinningCount - 1;
						}

						buckets[b].Count++;
						buckets[b].Bounds = AABB::Combine(buckets[b].Bounds, primitiveInfo[i].Bounds);
					}

					//Calculate the Surface Area Heuristic.
					std::vector<float> sahCosts(m_config.BinningCount - 1);
					for (int i = 0; i < m_config.BinningCount - 1; ++i)
					{
						AABB ba = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
						AABB bb = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
						int countA = 0;
						int countB = 0;

						for (int j = 0; j <= i; ++j)
						{
							ba = AABB::Combine(ba, buckets[j].Bounds);
							countA += buckets[j].Count;
						}

						for (int j = i + 1; j < m_config.BinningCount; ++j)
						{
							bb = AABB::Combine(bb, buckets[j].Bounds);
							countB += buckets[j].Count;
						}

						sahCosts[i] = m_config.TraversalCost + (countA * ba.GetSurfaceArea() + countB * bb.GetSurfaceArea()) / nodeBounds.GetSurfaceArea();
					}

					//Find the optimal SAH cost and utilize it.
					float minCost = sahCosts[0];
					int minCostSplitBucket = 0;
					for (int i = 1; i < m_config.BinningCount - 1; ++i)
					{
						if (sahCosts[i] < minCost)
						{
							minCost = sahCosts[i];
							minCostSplitBucket = i;
						}
					}

					//Either create a leaf node or split the primitives at the selected SAH bucket.
					int numPrimitives = end - start;
					if (numPrimitives > m_config.MaxPrimitiveCountInNode || minCost < numPrimitives)
					{
						BVHPrimitiveInfo* primitiveMid = std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
							[=](const BVHPrimitiveInfo& infoItem)
							{
								int b = static_cast<int>(m_config.BinningCount * centroidBounds.GetOffset(infoItem.Centroid)[sortAxis]);
								if (b == m_config.BinningCount)
								{
									b = m_config.BinningCount - 1;
								}
								return b <= minCostSplitBucket;
							});

						mid = static_cast<int>(std::distance(primitiveInfo.data(), primitiveMid));
					}
					else
					{
						//When only one primitive is available, create a leaf node with the given properties.
						int firstPrimitiveOffset = static_cast<int>(orderedPrimitives.size());
						for (int i = start; i < end; ++i)
						{
							orderedPrimitives.push_back(m_primitives[primitiveInfo[i].PrimitiveIndex]);
						}
						InitializeAsLeafNode(node, firstPrimitiveOffset, numPrimitives, nodeBounds);
						return node;
					}
					break;
				}
				default:
					printf("Undefined partition method!\n");
					break;
				}

				BVHBuildNode* leftChild = Build(primitiveInfo, start, mid, totalNodes, orderedPrimitives);
				BVHBuildNode* rightChild = Build(primitiveInfo, mid, end, totalNodes, orderedPrimitives);
				InitializeAsInteriorNode(node, sortAxis, leftChild, rightChild);
			}
		}

		return node;
	}

	BVH::SortAxis BVH::GetSortAxis(const AABB& centroidBounds, int start, int end)
	{
		glm::vec3 diagonal = centroidBounds.GetMax() - centroidBounds.GetMin();
		if (diagonal.x > diagonal.y && diagonal.x > diagonal.z)
		{
			return SortAxis::X;
		}
		else if (diagonal.y > diagonal.z)
		{
			return SortAxis::Y;
		}
		else
		{
			return SortAxis::Z;
		}
	}

	void BVH::InitializeAsLeafNode(BVHBuildNode* buildNode, int first, int numPrimitives, const AABB& bounds)
	{
		buildNode->Children[0] = nullptr;
		buildNode->Children[1] = nullptr;
		buildNode->FirstPrimitiveOffset = first;
		buildNode->Bounds = bounds;
		buildNode->NumPrimitives = numPrimitives;
	}

	void BVH::InitializeAsInteriorNode(BVHBuildNode* buildNode, SortAxis axis, BVHBuildNode* left, BVHBuildNode* right)
	{
		buildNode->Children[0] = left;
		buildNode->Children[1] = right;
		buildNode->Bounds = AABB::Combine(left->Bounds, right->Bounds);
		buildNode->SplitAxis = axis;
		buildNode->NumPrimitives = 0;
	}

	int BVH::FlattenBVHTree(BVHBuildNode* node, int& offset)
	{
		LinearBVHNode* linearNode = &m_nodes[offset];
		linearNode->Bounds = node->Bounds;
		int linearNodeOffset = offset++;

		if(node->NumPrimitives > 0)
		{
			//Leaf node.
			linearNode->PrimitiveOffset = node->FirstPrimitiveOffset;
			linearNode->NumPrimitives = node->NumPrimitives;
		}
		else
		{
			//Interior node.
			linearNode->Axis = node->SplitAxis;
			linearNode->NumPrimitives = 0;
			FlattenBVHTree(node->Children[0], offset);
			linearNode->SecondChildOffset = FlattenBVHTree(node->Children[1], offset);
		}

		return linearNodeOffset;
	}

	void BVH::Shutdown()
	{
		FreeAligned(m_nodes);
	}

	BVH::BVHPrimitiveInfo::BVHPrimitiveInfo() : 
		PrimitiveIndex(-1),
		Bounds(),
		Centroid(0.0f) 
	{};

	BVH::BVHPrimitiveInfo::BVHPrimitiveInfo(int primitiveNumber, const AABB& bounds) :
		PrimitiveIndex(primitiveNumber),
		Bounds(bounds),
		Centroid(bounds.GetCenter())
	{}
}