---
name: renderer-dev
description: Short guidance for implementing renderer features using RenderCore abstractions.
---

# Renderer Development

Use RenderCore as the foundation. Renderer code should build on it, not replace it.

## Core rules

- Shader parameter metadata is defined in `src/Framework/RenderCore/PublicHeader/ShaderParameter.h`.
- Use `BEGIN_SHADER_PARAMETER_STRUCT` / `END_SHADER_PARAMETER_STRUCT` for all shader parameter structs.
- Use `SHADER_PARAMETER(...)` for scalar/vector members and resource macros for textures, samplers, buffers, UAVs, etc.
- If a type is used in `SHADER_PARAMETER_RDG_STRUCTURED_BUFFER` or `SHADER_PARAMETER_RDG_RWSTRUCTURED_BUFFER`, it must be a proper shader-reflection struct with metadata, not a plain runtime struct.
- `TAlignedType` in `ShaderParameterTypeInfo<T>` tells the actual aligned storage type; check it when defining new parameter types.

## Workflow

1. Define shader parameter structs in RenderCore style.
2. Define corresponding shader classes.
3. Use the right shader base class:
   - Global shader: RenderCore global shader base
   - Material shader: renderer material shader base
   - Mesh material / vertex-factory shader: mesh material shader path
4. Add variant logic using shader static functions such as `ShouldCompilePermutation`, `ModifyShaderCompilerEnvironment`, and `GetShaderParameterMetadata`.
5. Bind resources and parameters to actual use.
6. Execute either by direct RHI command submission or through `RenderGraphBuilder` pass scheduling.

## Shader definitions

The project has a standard shader architecture:
- RenderCore defines the common shader abstraction and reflection system.
- `ShaderCore.h` contains the compile/variant domain logic.
- Renderer modules define concrete shaders on top of RenderCore.
- Mesh material shaders are the path for vertex-factory-based rendering.

Follow existing project patterns instead of inventing a new custom model.

## Execution modes

### Direct RHI execution

Use it for simpler standalone tasks.

Typical flow:
- get queue/context
- create command list
- bind pipeline state
- set shader parameters
- dispatch/draw
- flush or end command list

Reference: test shader parameter execution paths and direct feature code.

### RenderGraph execution

Use it for engine-level passes and resource-managed rendering.

Typical flow:
- add pass to `RenderGraphBuilder`
- create/access RDG resources and buffers
- define pass dependencies and resource transitions
- set parameters with the pass input data
- let the builder schedule the pass

Reference: renderer pass implementations and `DefferedSceneRenderer.cpp` style usage.

## Implementation guidance

When adding new renderer code:

- read `ShaderParameter.h` first
- use RenderCore abstractions rather than ad hoc low-level types
- keep shader metadata and runtime data separated when needed
- prefer engine conventions and existing shaders over custom patterns
- treat `RenderGraphBuilder` as the default orchestration layer for renderer passes

## Final rule

Renderer features should be implemented as a layer above RenderCore:

`shader metadata -> shader class -> shader variant -> parameter binding -> execute via RHI or RenderGraph`

This is the expected architecture for new renderer functionality.

## English vs Chinese for Copilot

- English is usually better for agent readability and consistency with code comments, shader naming, and project conventions.
- Chinese is fine if the project is entirely localized, but English is more robust because model training and instruction-following are stronger in English.
- Best practice: keep the project instruction file in English, but add short Chinese notes only where local team conventions need them.
