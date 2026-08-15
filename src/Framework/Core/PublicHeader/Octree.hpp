#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <set>
#include <vector>

#include "BVH.hpp"

namespace Core
{
    static constexpr uint32_t OctreeInvalidNode = UINT32_MAX;

    struct OctreeNode
    {
        BoxSphereBounds Bounds;

        std::array<uint32_t, 8> Children;

        uint32_t FirstPrimitive = 0;
        uint32_t PrimitiveCount = 0;

        bool IsLeaf = true;

        OctreeNode()
        {
            Children.fill(OctreeInvalidNode);
        }

        bool IsEmpty() const
        {
            return IsLeaf && PrimitiveCount == 0;
        }
    };

    template<typename Primitive>
    struct OctreeTraits
    {
        static BoxSphereBounds GetBounds(const Primitive& Primitive);

        static Float3 GetCentroid(const Primitive& Primitive);

        static bool RayIntersect(const Primitive& Primitive, const Ray& Ray, float& OutDistance);

        static Float3 ClosestPoint(const Primitive& Primitive, const Float3& Position);

        static float ClosestDistance(const Primitive& Primitive, const Float3& Position);
    };

    template<typename Primitive, typename Traits = OctreeTraits<Primitive>>
    class Octree
    {
    public:

        using PrimitiveType = Primitive;
        using TraitsType = Traits;

        Octree() = default;
        ~Octree() = default;

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

        const OctreeNode& GetRootNode() const
        {
            return Nodes.front();
        }

        const std::vector<OctreeNode>& GetNodes() const
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

        void SetMaxDepth(uint32_t InMaxDepth)
        {
            MaxDepth = InMaxDepth;
        }

        void SetMaxPrimitivesPerLeaf(uint32_t InMaxPrimitivesPerLeaf)
        {
            MaxPrimitivesPerLeaf = InMaxPrimitivesPerLeaf;
        }

        float ClosestDistance(const Float3& Position) const;

        void RayIntersect(const Ray& Ray, std::set<uint32_t>& IntersectPrimitiveIds) const;

        void GetPrimitiveIdsInsideBounds(const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const;

    protected:

        uint32_t BuildRecursive(uint32_t Begin, uint32_t End, const AABB& Bounds, uint32_t Depth);

        uint32_t CreateLeaf(uint32_t Begin, uint32_t End, const BoxSphereBounds& Bounds);

        uint32_t CreateInternalNode(const BoxSphereBounds& Bounds);

        BoxSphereBounds ComputeBounds(uint32_t Begin, uint32_t End) const;

        BoxSphereBounds ComputeBounds() const;

        static int GetChildIndex(const Float3& Position, const AABB& Bounds);

        static AABB GetChildBounds(const AABB& ParentBounds, uint32_t ChildIndex);

        void ClosestDistanceRecursive(uint32_t NodeIndex, const Float3& Position, float& MinDistance) const;

        void RayIntersectNode(
            uint32_t NodeIndex,
            const Ray& Ray,
            std::set<uint32_t>& OutPrimitiveIds) const;

        void QueryBoundsRecursive(uint32_t NodeIndex, const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const;

    protected:

        const std::vector<Primitive>* Primitives = nullptr;

        std::vector<uint32_t> PrimitiveIndices;

        std::vector<OctreeNode> Nodes;

        uint32_t MaxDepth = 8;
        uint32_t MaxPrimitivesPerLeaf = 8;
    };

    template<typename Primitive, typename Traits>
    void Octree<Primitive, Traits>::Build(const std::vector<Primitive>& InPrimitives)
    {
        Clear();

        if (InPrimitives.empty())
        {
            return;
        }

        Primitives = &InPrimitives;

        PrimitiveIndices.resize(InPrimitives.size());

        for (uint32_t i = 0; i < static_cast<uint32_t>(InPrimitives.size()); i++)
        {
            PrimitiveIndices[i] = i;
        }

        Nodes.reserve(InPrimitives.size() * 8);

        const BoxSphereBounds RootBounds = ComputeBounds();

        if (RootBounds.Box.IsEmpty())
        {
            return;
        }

        BuildRecursive(0, static_cast<uint32_t>(InPrimitives.size()), RootBounds.Box, 0);
    }

    template<typename Primitive, typename Traits>
    uint32_t Octree<Primitive, Traits>::BuildRecursive(uint32_t Begin, uint32_t End, const AABB& Bounds, uint32_t Depth)
    {
        const uint32_t PrimitiveCount = End - Begin;

        if (PrimitiveCount <= MaxPrimitivesPerLeaf || Depth >= MaxDepth)
        {
            return CreateLeaf(Begin, End, BoxSphereBounds(Bounds));
        }

        std::array<std::vector<uint32_t>, 8> ChildPrimitives;

        for (uint32_t i = Begin; i < End; i++)
        {
            uint32_t PrimitiveIndex = PrimitiveIndices[i];
            const Primitive& PrimitiveValue = (*Primitives)[PrimitiveIndex];
            const Float3 PrimitiveCenter = Traits::GetCentroid(PrimitiveValue);
            const int ChildIndex = GetChildIndex(PrimitiveCenter, Bounds);
            ChildPrimitives[ChildIndex].push_back(PrimitiveIndex);
        }

        bool HasAnyChild = false;

        for (uint32_t i = 0; i < 8; i++)
        {
            if (!ChildPrimitives[i].empty())
            {
                HasAnyChild = true;
                break;
            }
        }

        if (!HasAnyChild)
        {
            return CreateLeaf(Begin, End, BoxSphereBounds(Bounds));
        }

        uint32_t NodeIndex = CreateInternalNode(BoxSphereBounds(Bounds));
        OctreeNode& Node = Nodes[NodeIndex];

        uint32_t WriteIndex = Begin;

        for (uint32_t ChildIndex = 0; ChildIndex < 8; ChildIndex++)
        {
            if (ChildPrimitives[ChildIndex].empty())
            {
                Node.Children[ChildIndex] = OctreeInvalidNode;
                continue;
            }

            const uint32_t ChildBegin = WriteIndex;
            const uint32_t ChildEnd = ChildBegin + static_cast<uint32_t>(ChildPrimitives[ChildIndex].size());

            for (uint32_t PrimitiveIndex : ChildPrimitives[ChildIndex])
            {
                PrimitiveIndices[WriteIndex++] = PrimitiveIndex;
            }

            const AABB ChildBounds = GetChildBounds(Bounds, ChildIndex);
            Node.Children[ChildIndex] = BuildRecursive(ChildBegin, ChildEnd, ChildBounds, Depth + 1);
        }

        Node.IsLeaf = false;

        return NodeIndex;
    }

    template<typename Primitive, typename Traits>
    uint32_t Octree<Primitive, Traits>::CreateLeaf(uint32_t Begin, uint32_t End, const BoxSphereBounds& Bounds)
    {
        OctreeNode Node;

        Node.Bounds = Bounds;
        Node.FirstPrimitive = Begin;
        Node.PrimitiveCount = End - Begin;
        Node.IsLeaf = true;
        Node.Children.fill(OctreeInvalidNode);

        Nodes.push_back(Node);

        return static_cast<uint32_t>(Nodes.size() - 1);
    }

    template<typename Primitive, typename Traits>
    uint32_t Octree<Primitive, Traits>::CreateInternalNode(const BoxSphereBounds& Bounds)
    {
        OctreeNode Node;

        Node.Bounds = Bounds;
        Node.IsLeaf = false;
        Node.Children.fill(OctreeInvalidNode);

        Nodes.push_back(Node);

        return static_cast<uint32_t>(Nodes.size() - 1);
    }

    template<typename Primitive, typename Traits>
    BoxSphereBounds Octree<Primitive, Traits>::ComputeBounds(uint32_t Begin, uint32_t End) const
    {
        BoxSphereBounds Bounds;
        Bounds.SetEmpty();

        for (uint32_t i = Begin; i < End; i++)
        {
            const Primitive& PrimitiveValue = (*Primitives)[PrimitiveIndices[i]];
            Bounds.Merge(Traits::GetBounds(PrimitiveValue));
        }

        return Bounds;
    }

    template<typename Primitive, typename Traits>
    BoxSphereBounds Octree<Primitive, Traits>::ComputeBounds() const
    {
        BoxSphereBounds Bounds;
        Bounds.SetEmpty();

        for (const Primitive& PrimitiveValue : *Primitives)
        {
            Bounds.Merge(Traits::GetBounds(PrimitiveValue));
        }

        return Bounds;
    }

    template<typename Primitive, typename Traits>
    int Octree<Primitive, Traits>::GetChildIndex(const Float3& Position, const AABB& Bounds)
    {
        const Float3 Center = Bounds.GetCenter();

        int ChildIndex = 0;

        if (Position.x >= Center.x)
        {
            ChildIndex |= 1;
        }

        if (Position.y >= Center.y)
        {
            ChildIndex |= 2;
        }

        if (Position.z >= Center.z)
        {
            ChildIndex |= 4;
        }

        return ChildIndex;
    }

    template<typename Primitive, typename Traits>
    AABB Octree<Primitive, Traits>::GetChildBounds(const AABB& ParentBounds, uint32_t ChildIndex)
    {
        const Float3 Center = ParentBounds.GetCenter();

        Float3 Min = ParentBounds.Min;
        Float3 Max = ParentBounds.Max;

        if ((ChildIndex & 1) != 0)
        {
            Min.x = Center.x;
        }
        else
        {
            Max.x = Center.x;
        }

        if ((ChildIndex & 2) != 0)
        {
            Min.y = Center.y;
        }
        else
        {
            Max.y = Center.y;
        }

        if ((ChildIndex & 4) != 0)
        {
            Min.z = Center.z;
        }
        else
        {
            Max.z = Center.z;
        }

        return AABB(Min, Max);
    }

    template<typename Primitive, typename Traits>
    float Octree<Primitive, Traits>::ClosestDistance(const Float3& Position) const
    {
        float MinDistance = std::numeric_limits<float>::max();

        if (Nodes.empty())
        {
            return MinDistance;
        }

        ClosestDistanceRecursive(0, Position, MinDistance);

        return MinDistance;
    }

    template<typename Primitive, typename Traits>
    void Octree<Primitive, Traits>::ClosestDistanceRecursive(uint32_t NodeIndex, const Float3& Position, float& MinDistance) const
    {
        const OctreeNode& Node = Nodes[NodeIndex];

        float BoundsDistance = Node.Bounds.Box.Distance(Position);

        if (BoundsDistance > MinDistance)
        {
            return;
        }

        if (Node.IsLeaf)
        {
            for (uint32_t i = 0; i < Node.PrimitiveCount; i++)
            {
                uint32_t PrimitiveIndex = PrimitiveIndices[Node.FirstPrimitive + i];
                float Distance = Traits::ClosestDistance((*Primitives)[PrimitiveIndex], Position);
                MinDistance = std::min(MinDistance, Distance);
            }
            return;
        }

        for (uint32_t i = 0; i < 8; i++)
        {
            if (Node.Children[i] != OctreeInvalidNode)
            {
                ClosestDistanceRecursive(Node.Children[i], Position, MinDistance);
            }
        }
    }

    template<typename Primitive, typename Traits>
    void Octree<Primitive, Traits>::RayIntersect(const Ray& Ray, std::set<uint32_t>& IntersectPrimitiveIds) const
    {
        IntersectPrimitiveIds.clear();

        if (Nodes.empty())
        {
            return;
        }

        RayIntersectNode(0, Ray, IntersectPrimitiveIds);
    }

    template<typename Primitive, typename Traits>
    void Octree<Primitive, Traits>::RayIntersectNode(uint32_t NodeIndex, const Ray& Ray, std::set<uint32_t>& OutPrimitiveIds) const
    {
        const OctreeNode& Node = Nodes[NodeIndex];

        if (!Ray.RayIntersectAABB(Node.Bounds.Box))
        {
            return;
        }

        if (Node.IsLeaf)
        {
            for (uint32_t i = 0; i < Node.PrimitiveCount; i++)
            {
                uint32_t PrimitiveIndex = PrimitiveIndices[Node.FirstPrimitive + i];
                float Distance = 0.0f;
                if (Traits::RayIntersect((*Primitives)[PrimitiveIndex], Ray, Distance))
                {
                    OutPrimitiveIds.insert(PrimitiveIndex);
                }
            }
            return;
        }

        for (uint32_t i = 0; i < 8; i++)
        {
            if (Node.Children[i] != OctreeInvalidNode)
            {
                RayIntersectNode(Node.Children[i], Ray, OutPrimitiveIds);
            }
        }
    }

    template<typename Primitive, typename Traits>
    void Octree<Primitive, Traits>::GetPrimitiveIdsInsideBounds(const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const
    {
        OutPrimitiveIds.clear();

        if (Nodes.empty())
        {
            return;
        }

        QueryBoundsRecursive(0, Box, OutPrimitiveIds);
    }

    template<typename Primitive, typename Traits>
    void Octree<Primitive, Traits>::QueryBoundsRecursive(uint32_t NodeIndex, const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const
    {
        const OctreeNode& Node = Nodes[NodeIndex];

        if (!Node.Bounds.Intersects(Box))
        {
            return;
        }

        if (Node.IsLeaf)
        {
            for (uint32_t i = 0; i < Node.PrimitiveCount; i++)
            {
                uint32_t PrimitiveIndex = PrimitiveIndices[Node.FirstPrimitive + i];
                OutPrimitiveIds.insert(PrimitiveIndex);
            }
            return;
        }

        for (uint32_t i = 0; i < 8; i++)
        {
            if (Node.Children[i] != OctreeInvalidNode)
            {
                QueryBoundsRecursive(Node.Children[i], Box, OutPrimitiveIds);
            }
        }
    }
}
