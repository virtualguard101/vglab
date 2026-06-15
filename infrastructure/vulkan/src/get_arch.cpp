// get_arch.cpp — 枚举物理 GPU 并打印设备属性
// 对比CUDA：
//   cudaGetDeviceCount        → vkEnumeratePhysicalDevices
//   cudaGetDeviceProperties   → vkGetPhysicalDeviceProperties
//   props.totalGlobalMem 等   → memory heaps + VkPhysicalDeviceLimits
//
// 本程序只用到 Vulkan 最顶层两步：Instance → PhysicalDevice。
// 不涉及 LogicalDevice、Queue、Swapchain、Pipeline（那些是后续渲染/计算才需要的）。

#include <vulkan/vulkan.h>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Vulkan 几乎所有函数都返回 VkResult（成功为 VK_SUCCESS）。
// 宏封装后可在教学代码里统一处理错误，避免每个调用都写 if 判断。
#define VK_CHECK(expr)                                                         \
  do {                                                                         \
    VkResult _result = (expr);                                                 \
    if (_result != VK_SUCCESS) {                                               \
      throw std::runtime_error(std::string("Vulkan error: ") +                 \
                               std::to_string(_result));                       \
    }                                                                          \
  } while (0)

// VkPhysicalDeviceType 把设备粗分为独显/核显/虚拟 GPU 等。
// 注意：类型不影响 limits 的“规格上限”，但能帮你看清是不是核显（共享内存）。
const char* device_type_name(VkPhysicalDeviceType type) {
  switch (type) {
  case VK_PHYSICAL_DEVICE_TYPE_OTHER:
    return "Other";
  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    return "Integrated GPU";
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    return "Discrete GPU";
  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    return "Virtual GPU";
  case VK_PHYSICAL_DEVICE_TYPE_CPU:
    return "CPU";
  default:
    return "Unknown";
  }
}

// 查询单个物理设备的静态属性（名称、厂商 ID、API 版本、limits 等）。
// VkPhysicalDevice 是“显卡硬件”的句柄，创建后无需销毁（由 Instance 管理生命周期）。
VkPhysicalDeviceProperties get_device_properties(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties props{};
  vkGetPhysicalDeviceProperties(device, &props);
  return props;
}

// 查询内存布局：heap（堆，有 size）+ type（类型，指向某个 heap）。
// 类比 CUDA 的 totalGlobalMem，但 Vulkan 把“能放 GPU 数据的空间”拆成多个 heap。
VkPhysicalDeviceMemoryProperties get_memory_properties(VkPhysicalDevice device) {
  VkPhysicalDeviceMemoryProperties mem_props{};
  vkGetPhysicalDeviceMemoryProperties(device, &mem_props);
  return mem_props;
}

// 累加所有 DEVICE_LOCAL heap 的容量，近似 CUDA 的 totalGlobalMem。
// 核显上这块内存通常来自系统 RAM 划分，不是独立 GDDR 颗粒。
uint64_t device_local_memory_bytes(
    const VkPhysicalDeviceMemoryProperties& mem_props) {
  uint64_t total = 0;
  for (uint32_t i = 0; i < mem_props.memoryHeapCount; ++i) {
    if (mem_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
      total += mem_props.memoryHeaps[i].size;
    }
  }
  return total;
}

int main() {
  // -------------------------------------------------------------------------
  // 1. 填写 VkApplicationInfo（可选元数据，loader/验证层可用来识别应用）
  // -------------------------------------------------------------------------
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;  // Vulkan 结构体惯例：必须设 sType
  app_info.pApplicationName = "get_arch";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "vglab";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  // 请求的 Vulkan API 版本；驱动可支持更高版本，实际能力见 props.apiVersion
  app_info.apiVersion = VK_API_VERSION_1_0;

  // -------------------------------------------------------------------------
  // 2. 创建 VkInstance —— Vulkan 入口，连接应用与 Vulkan loader/驱动
  // -------------------------------------------------------------------------
  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  // 完整应用还会在这里配置：
  //   - ppEnabledExtensionNames（如 VK_KHR_surface 用于窗口）
  //   - ppEnabledLayerNames（验证层，调试时用）
  // 本 demo 不创建窗口，因此零扩展即可枚举物理设备。

  VkInstance instance = VK_NULL_HANDLE;
  try {
    // 第二个参数是 VkAllocationCallbacks*（自定义内存分配器），nullptr 用默认分配
    VK_CHECK(vkCreateInstance(&instance_info, nullptr, &instance));

    // -----------------------------------------------------------------------
    // 3. 枚举物理设备 —— Vulkan 典型的“两次调用”模式
    // -----------------------------------------------------------------------
    // 第一次：device_count 传有效指针，devices 传 nullptr → 只获取数量
    uint32_t device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));

    std::cout << "Found " << device_count << " physical device(s)\n\n";

    if (device_count == 0) {
      vkDestroyInstance(instance, nullptr);
      return 0;
    }

    // 第二次：传入预分配数组，把 VkPhysicalDevice 句柄写回来
    std::vector<VkPhysicalDevice> devices(device_count);
    VK_CHECK(
        vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

    // -----------------------------------------------------------------------
    // 4. 遍历每台物理设备，打印属性
    // -----------------------------------------------------------------------
    for (uint32_t i = 0; i < device_count; ++i) {
      VkPhysicalDeviceProperties props = get_device_properties(devices[i]);
      VkPhysicalDeviceMemoryProperties mem_props =
          get_memory_properties(devices[i]);
      // limits 里是各类“上限”，类似 CUDA 的 maxThreadsPerBlock / maxGridSize。
      // 注意：这些大多是 API/规范层面的最大值，不能直接用来横向比较 GPU 算力。
      const VkPhysicalDeviceLimits& limits = props.limits;

      std::cout << "-----------GPU " << i << "-----------\n";
      std::cout << "GPU " << i << ": " << props.deviceName << '\n';
      std::cout << "Device type: " << device_type_name(props.deviceType) << '\n';

      // vendorID: 0x1002=AMD, 0x10DE=NVIDIA, 0x8086=Intel
      std::string vendor_name = [&props]() {
        switch (props.vendorID) {
          case 0x1002:
            return "AMD";
          case 0x10DE:
            return "NVIDIA";
          case 0x8086:
            return "Intel";
          default:
            return "Unknown";
        }
      }();
      std::cout << "Vendor ID: 0x" << std::hex << props.vendorID << std::dec << " (" << vendor_name << ")"
                << '\n';
      std::cout << "Device ID: 0x" << std::hex << props.deviceID << std::dec
                << '\n';

      // 驱动实现的 Vulkan API 版本（与 app_info.apiVersion 不同）
      std::cout << "Vulkan API version: " << VK_VERSION_MAJOR(props.apiVersion)
                << '.' << VK_VERSION_MINOR(props.apiVersion) << '.'
                << VK_VERSION_PATCH(props.apiVersion) << '\n';

      // 驱动版本编码因厂商而异：AMD/NVIDIA 常把版本信息塞进 driverVersion
      std::cout << "Driver version: " << VK_VERSION_MAJOR(props.driverVersion)
                << '.' << VK_VERSION_MINOR(props.driverVersion) << '.'
                << VK_VERSION_PATCH(props.driverVersion) << '\n';
      std::cout << "Device-local memory: "
                << device_local_memory_bytes(mem_props) << " bytes\n";

      // --- 计算着色器相关 limits（对标 CUDA block/grid 维度上限）---
      // maxComputeSharedMemorySize ≈ CUDA sharedMemPerBlock
      std::cout << "Max compute shared memory: "
                << limits.maxComputeSharedMemorySize << " bytes\n";
      // maxComputeWorkGroupInvocations ≈ CUDA maxThreadsPerBlock
      std::cout << "Max compute work group invocations: "
                << limits.maxComputeWorkGroupInvocations << '\n';
      // work group 在 x/y/z 各维度的最大线程数
      std::cout << "Max compute work group size: "
                << limits.maxComputeWorkGroupSize[0] << 'x'
                << limits.maxComputeWorkGroupSize[1] << 'x'
                << limits.maxComputeWorkGroupSize[2] << '\n';
      // dispatch 时各维度最多能 launch 多少个 work group ≈ CUDA maxGridSize
      std::cout << "Max compute work group count: "
                << limits.maxComputeWorkGroupCount[0] << 'x'
                << limits.maxComputeWorkGroupCount[1] << 'x'
                << limits.maxComputeWorkGroupCount[2] << '\n';

      // 单个 storage buffer 可寻址的最大字节数（规范上限，常是 2^32-1）
      std::cout << "Max storage buffer range: "
                << limits.maxStorageBufferRange << " bytes\n";
      // push constants：小块常量数据，经命令直接推给 shader，无需 descriptor set
      std::cout << "Max push constants size: "
                << limits.maxPushConstantsSize << " bytes\n";
      if (i + 1 < device_count) {
        std::cout << '\n';
      }
    }

    // Instance 由应用创建，必须显式销毁（RAII 包装类在 larger 项目里更常见）
    vkDestroyInstance(instance, nullptr);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    if (instance != VK_NULL_HANDLE) {
      vkDestroyInstance(instance, nullptr);
    }
    return 1;
  }

  return 0;
}
