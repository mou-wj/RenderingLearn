#pragma once

#include <vector>
#include <cstdint>
#include <set>
#include "Common.h"
#include "BoxSphereBounds.h"

namespace Core
{

    struct Ray {
		Float3 Origin;
		Float3 Direction;
		Ray() = default;
		Ray(const Float3& InOrigin, const Float3& InDirection)
			: Origin(InOrigin), Direction(InDirection) {
		}
        bool RayIntersectAABB(
            const AABB& Box) const
        {

            
            float tMin = 0.0f;
            float tMax = FLT_MAX;


            for (int Axis = 0; Axis < 3; Axis++)
            {
                float Ori = Origin[Axis];
                float Dir = Direction[Axis];


                float InvD = 1.0f / Dir;


                float t0 =
                    (Box.Min[Axis] - Ori) * InvD;


                float t1 =
                    (Box.Max[Axis] - Ori) * InvD;


                if (t0 > t1)
                {
                    std::swap(t0, t1);
                }


                tMin = std::max(tMin, t0);
                tMax = std::min(tMax, t1);


                if (tMax < tMin)
                {
                    return false;
                }
            }


            return true;
        }
    };

    static constexpr uint32_t BVHInvalidNode = UINT32_MAX;

    struct BVHNode
    {
        BoxSphereBounds Bounds;

        uint32_t LeftChild = BVHInvalidNode;
        uint32_t RightChild = BVHInvalidNode;

        uint32_t FirstPrimitive = 0;
        uint32_t PrimitiveCount = 0;

        bool IsLeaf() const { return LeftChild == BVHInvalidNode; }
    };

    template<typename Primitive>
    struct BVHTraits
    {
        static BoxSphereBounds GetBounds(const Primitive& Primitive);

        static Float3 GetCentroid(const Primitive& Primitive);

        static bool RayIntersect(const Primitive& Primitive, const Ray& Ray, float& OutDistance);

        static Float3 ClosestPoint(const Primitive& Primitive, const Float3& Position);

        static float ClosestDistance(const Primitive& Primitive, const Float3& Position);
    };

    template<typename Primitive, typename Traits = BVHTraits<Primitive>>
    class BVH
    {
    public:

        using PrimitiveType = Primitive;
        using TraitsType = Traits;

        BVH() = default;
        ~BVH() = default;

        void Build(const std::vector<Primitive>& InPrimitives);

        void Clear()
        {
            Nodes.clear();
            PrimitiveIndices.clear();
            Primitives = nullptr;
        }

        bool IsEmpty() const
        {
            return Nodes.empty();
        }

        const BVHNode& GetRootNode() const
        {
            return Nodes.front();
        }

        const std::vector<BVHNode>& GetNodes() const
        {
            return Nodes;
        }

        const std::vector<uint32_t>& GetPrimitiveIndices() const
        {
            return PrimitiveIndices;
        }

        const Primitive& GetPrimitive(uint32_t Index) const
        {
            return (*Primitives)[Index];
        }

        const std::vector<Primitive>& GetPrimitives() const
        {
            return *Primitives;
        }

        uint32_t GetNodeCount() const
        {
            return static_cast<uint32_t>(Nodes.size());
        }

        uint32_t GetPrimitiveCount() const
        {
            return static_cast<uint32_t>(PrimitiveIndices.size());
        }

        float ClosestDistance(const Float3& Position) const;

        void RayIntersect(const Ray& Ray, std::set<uint32_t>& IntersectPrimitiveIds) const;

        void GetPrimitiveIdsInsideBounds(const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const;

    protected:

        uint32_t BuildRecursive(uint32_t Begin, uint32_t End);

        uint32_t CreateLeaf(uint32_t Begin, uint32_t End);

        uint32_t CreateInternalNode();

        BoxSphereBounds ComputeBounds(uint32_t Begin, uint32_t End) const;

        int GetLongestAxis(const BoxSphereBounds& Bounds) const;

        void ClosestDistanceRecursive(uint32_t NodeIndex, const Float3& Position, float& MinDistance) const;

        void RayIntersectNode(
            uint32_t NodeIndex,
            const Ray& Ray,
            std::set<uint32_t>& OutPrimitiveIds) const;

        void QueryBoundsRecursive(uint32_t NodeIndex, const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const;
    protected:




        const std::vector<Primitive>* Primitives = nullptr;

        std::vector<uint32_t> PrimitiveIndices;

        std::vector<BVHNode> Nodes;
    };

}
#include "BVH.inl"