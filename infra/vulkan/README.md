# vulkan

Minimal Vulkan experiments for `vglab`.

**Maturity**: `teaching` / `poc`

## Dependencies

- C++17 compiler
- [Vulkan loader + headers](https://vulkan.lunarg.com/) (Arch: `vulkan-devel`)

## Build & run

```bash
just build
just run
```

## Programs

| Source | Description |
|--------|-------------|
| `src/get_arch.cpp` | Enumerate physical devices and print properties (similar to `cuda-learning/src/get_arch.cu`) |
