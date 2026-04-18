#pragma once
#include <vec3.hpp>
#include <memory>
#include <vector>
#include "primitives/primitive.hpp"

namespace CRT
{
	class AABB;
	struct Ray;
	struct HitInfo;

	/**
	 * @brief Used to select the BVH build algorithm.
	 */
	enum class PartitionType : uint8_t
	{
		Middle = 0,
		EqualCounts,
		SurfaceAreaHeuristic
	};

	/**
	 * @brief The BVH can be configured for different partition results, which can adjust the performance of the scene.
	 */
	struct BVHConfig final
	{
		float TraversalCost = 0.35f;
		float IntersectionCost = 1;
		int BinningCount = 16;
		int MaxPrimitiveCountInNode = 1;
		PartitionType PartitionType = PartitionType::SurfaceAreaHeuristic;
	};
	
	/**
	 * @brief Represents a Boundary Volume Hierarchy based on the pbr-book.org implementation, this serves as a base class and constructs only through the means of SAH/middle/equal count.
	 * https://pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies
	 */
	class BVH final : public Primitive
	{
	public:
		BVH();
		~BVH() = default;

		virtual void Initialize(const BVHConfig& config, const std::vector<std::shared_ptr<Primitive>>& primitives);
		virtual bool Intersect(const Ray& ray, HitInfo& lastHitInfo, const float maxRayLength) override;

		virtual void Shutdown();

	private:
		enum SortAxis : uint8_t
		{
			X = 0,
			Y, 
			Z
		};

		/**
		 * @brief Used during the binning phased of the SAH construction of the BVH.
		 */
		struct BucketInfo final
		{
			int Count = 0;
			AABB Bounds = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
		};

		/**
		 * @brief Used to identify primitve nodes inside the BVH tree.
		 */
		struct BVHPrimitiveInfo final
		{
			int PrimitiveIndex;
			AABB Bounds;
			glm::vec3 Centroid;

			BVHPrimitiveInfo();
			BVHPrimitiveInfo(int primitiveNumber, const AABB& bounds);
		};

		/**
		 * @brief Used to identify build nodes within the BVH tree.
		 */
		struct BVHBuildNode final
		{
			AABB Bounds;
			BVHBuildNode* Children[2] = { nullptr, nullptr };
			SortAxis SplitAxis;
			int FirstPrimitiveOffset, NumPrimitives;
		};

		/**
		 * @brief Used to traverse over the BVH in a cache friendly manner.
		 */
		struct LinearBVHNode final
		{
			AABB Bounds;
			union
			{
				int PrimitiveOffset;	/* Leaf node */
				int SecondChildOffset;	/* Interior Node */
			};

			uint16_t NumPrimitives;
			uint16_t Axis;
		};

		void InitializeAsLeafNode(BVHBuildNode* node, int first, int numPrimitives, const AABB& bounds);
		void InitializeAsInteriorNode(BVHBuildNode* buildNode, SortAxis axis, BVHBuildNode* left, BVHBuildNode* right);
		int FlattenBVHTree(BVHBuildNode* node, int& offset);

		BVH::BVHBuildNode* Build(std::vector<BVH::BVHPrimitiveInfo>& primitiveInfo, int start, int end, int& totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives);
		SortAxis GetSortAxis(const AABB& centroidBounds, int start, int end);

		LinearBVHNode* m_nodes;
		std::vector<std::shared_ptr<Primitive>> m_primitives;

		BVHConfig m_config;
	};
}