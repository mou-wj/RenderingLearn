#pragma once

#include <algorithm>

namespace Core
{

template<typename Primitive, typename Traits>
void BVH<Primitive, Traits>::Build(const std::vector<Primitive>& InPrimitives)
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

    Nodes.reserve(InPrimitives.size() * 2);

    BuildRecursive(0, static_cast<uint32_t>(PrimitiveIndices.size()));
}



template<typename Primitive, typename Traits>
uint32_t BVH<Primitive, Traits>::BuildRecursive(uint32_t Begin, uint32_t End)
{
    const uint32_t PrimitiveCount = End - Begin;


    constexpr uint32_t LeafPrimitiveCount = 4;


    if (PrimitiveCount <= LeafPrimitiveCount)
    {
        return CreateLeaf(Begin, End);
    }


    BoxSphereBounds Bounds = ComputeBounds(Begin, End);


    int Axis = GetLongestAxis(Bounds);


    uint32_t Middle = Begin + PrimitiveCount / 2;


    std::nth_element(
        PrimitiveIndices.begin() + Begin,
        PrimitiveIndices.begin() + Middle,
        PrimitiveIndices.begin() + End,
        [&](uint32_t A, uint32_t B)
        {
            Float3 CenterA =
                Traits::GetCentroid((*Primitives)[A]);

            Float3 CenterB =
                Traits::GetCentroid((*Primitives)[B]);


            float ValueA = 0.0f;
            float ValueB = 0.0f;


            if (Axis == 0)
            {
                ValueA = CenterA.x;
                ValueB = CenterB.x;
            }
            else if (Axis == 1)
            {
                ValueA = CenterA.y;
                ValueB = CenterB.y;
            }
            else
            {
                ValueA = CenterA.z;
                ValueB = CenterB.z;
            }


            return ValueA < ValueB;
        });


    uint32_t NodeIndex = CreateInternalNode();


    uint32_t LeftChild =
        BuildRecursive(Begin, Middle);


    uint32_t RightChild =
        BuildRecursive(Middle, End);



    BVHNode& Node = Nodes[NodeIndex];


    Node.LeftChild = LeftChild;
    Node.RightChild = RightChild;


    Node.Bounds = Nodes[LeftChild].Bounds;

    Node.Bounds.Merge(
        Nodes[RightChild].Bounds);



    return NodeIndex;
}



template<typename Primitive, typename Traits>
uint32_t BVH<Primitive, Traits>::CreateLeaf(
    uint32_t Begin,
    uint32_t End)
{
    BVHNode Node;


    Node.Bounds =
        ComputeBounds(Begin, End);


    Node.LeftChild = BVHInvalidNode;
    Node.RightChild = BVHInvalidNode;


    Node.FirstPrimitive = Begin;
    Node.PrimitiveCount = End - Begin;


    Nodes.push_back(Node);


    return static_cast<uint32_t>(Nodes.size() - 1);
}



template<typename Primitive, typename Traits>
uint32_t BVH<Primitive, Traits>::CreateInternalNode()
{
    BVHNode Node;

    Node.LeftChild = BVHInvalidNode;
    Node.RightChild = BVHInvalidNode;

    Nodes.push_back(Node);

    return static_cast<uint32_t>(Nodes.size() - 1);
}



template<typename Primitive, typename Traits>
BoxSphereBounds BVH<Primitive, Traits>::ComputeBounds(
    uint32_t Begin,
    uint32_t End) const
{
    BoxSphereBounds Bounds;

    Bounds.SetEmpty();


    for (uint32_t i = Begin; i < End; i++)
    {
        const Primitive& PrimitiveI =
            (*Primitives)[PrimitiveIndices[i]];


        Bounds.Merge(
            Traits::GetBounds(PrimitiveI));
    }


    return Bounds;
}



template<typename Primitive, typename Traits>
int BVH<Primitive, Traits>::GetLongestAxis(
    const BoxSphereBounds& Bounds) const
{
    Float3 Extent =
        Bounds.Box.GetExtent();


    if (Extent.x > Extent.y &&
        Extent.x > Extent.z)
    {
        return 0;
    }


    if (Extent.y > Extent.z)
    {
        return 1;
    }


    return 2;
}

template<typename Primitive, typename Traits>
float BVH<Primitive,Traits>::ClosestDistance(const Float3& Position) const
{
    float MinDistance = std::numeric_limits<float>::max();

    ClosestDistanceRecursive(
        0,
        Position,
        MinDistance);

    return MinDistance;
}
template<typename Primitive, typename Traits>
void BVH<Primitive,Traits>::ClosestDistanceRecursive(
    uint32_t NodeIndex,
    const Float3& Position,
    float& MinDistance) const
{
    const BVHNode& Node = Nodes[NodeIndex];


    float BoundsDistance =
        Node.Bounds.Box.Distance(Position);


    if (BoundsDistance > MinDistance)
    {
        return;
    }



    if (Node.IsLeaf())
    {
        for(uint32_t i=0;i<Node.PrimitiveCount;i++)
        {
            uint32_t PrimitiveIndex =
                PrimitiveIndices[
                    Node.FirstPrimitive+i];


            float Distance =
                Traits::ClosestDistance(
                    (*Primitives)[PrimitiveIndex],
                    Position);


            MinDistance =
                std::min(
                    MinDistance,
                    Distance);
        }

        return;
    }



    ClosestDistanceRecursive(
        Node.LeftChild,
        Position,
        MinDistance);


    ClosestDistanceRecursive(
        Node.RightChild,
        Position,
        MinDistance);
}

template<typename Primitive, typename Traits>
void BVH<Primitive, Traits>::RayIntersect(
    const Ray& Ray,
    std::set<uint32_t>& IntersectPrimitiveIds) const
{
    IntersectPrimitiveIds.clear();

    if (Nodes.empty())
    {
        return;
    }

    RayIntersectNode(
        0,
        Ray,
        IntersectPrimitiveIds);
}
template<typename Primitive, typename Traits>
void BVH<Primitive, Traits>::RayIntersectNode(
    uint32_t NodeIndex,
    const Ray& Ray,
    std::set<uint32_t>& OutPrimitiveIds) const
{
    const BVHNode& Node = Nodes[NodeIndex];


    if (!Ray.RayIntersectAABB(Node.Bounds.Box))
    {
        return;
    }


    if (Node.IsLeaf())
    {
        for (uint32_t i = 0; i < Node.PrimitiveCount; i++)
        {
            uint32_t InternalIndex =
                Node.FirstPrimitive + i;


            uint32_t PrimitiveId =
                PrimitiveIndices[InternalIndex];
            float dis = 0;
            if (Traits::RayIntersect((*Primitives)[PrimitiveId], Ray, dis)) {
                OutPrimitiveIds.insert(
                    PrimitiveId);
            }

        }

        return;
    }


    RayIntersectNode(
        Node.LeftChild,
        Ray,
        OutPrimitiveIds);


    RayIntersectNode(
        Node.RightChild,
        Ray,
        OutPrimitiveIds);
}

template<typename Primitive, typename Traits>
void BVH<Primitive, Traits>::GetPrimitiveIdsInsideBounds(const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const
{
    OutPrimitiveIds.clear();

    if (Nodes.empty())
    {
        return;
    }

    QueryBoundsRecursive(0, Box, OutPrimitiveIds);
}


template<typename Primitive, typename Traits>
void BVH<Primitive, Traits>::QueryBoundsRecursive(uint32_t NodeIndex, const BoxSphereBounds& Box, std::set<uint32_t>& OutPrimitiveIds) const
{
    const BVHNode& Node = Nodes[NodeIndex];


    if (!Node.Bounds.Intersects(Box))
    {
        return;
    }


    if (Node.IsLeaf())
    {
        for (uint32_t i = 0; i < Node.PrimitiveCount; i++)
        {
            uint32_t PrimitiveIndex = PrimitiveIndices[Node.FirstPrimitive + i];

            OutPrimitiveIds.insert(PrimitiveIndex);
        }

        return;
    }


    if (Node.LeftChild != BVHInvalidNode)
    {
        QueryBoundsRecursive(Node.LeftChild, Box, OutPrimitiveIds);
    }


    if (Node.RightChild != BVHInvalidNode)
    {
        QueryBoundsRecursive(Node.RightChild, Box, OutPrimitiveIds);
    }
}

}