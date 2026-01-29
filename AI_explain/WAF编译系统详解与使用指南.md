# ArduPilot WAF 编译系统详解与使用指南
./waf configure --board px4-v2

./waf --targets bin/arducopter --upload

1.根目录的waf脚本
d = p.dirname(p.realpath(__file__))
waf_light = p.join(d, 'modules', 'waf', 'waf-light')
try:
    subprocess.check_call(['python', waf_light] + sys.argv[1:])

waf文件所在目录和waf-light脚本目录（waf包括d, 'modules', 'waf', 'waf-light'四个目录）

2.根据waf构架
$ tree
|-- src
|   `-- wscript
`-- wscript
(1)先运行根目录的wscript（Python脚本）
(2)根据configure命令找configure函数（def定义的函数），完成是对编译工具的审查，看编译信息可知。
(3)./waf --targets bin/arducopter 其实是
./waf build --targets bin/arducopter
调用顶层wscript中的build函数
“--”代表参数option，调用option函数，参数为target 
之后就跳到tools/ardupilotwaf.py中的build了。

3.APM的编译层次：顶层是waf，涉及到pix的还是cmake。
## 概述

ArduPilot **不使用CMake**，而使用 **WAF** (Waftools) 构建系统。WAF是基于Python的高效构建工具，具有完美的依赖跟踪和增量编译能力，特别适合嵌入式开发。

---

## 为什么使用 WAF 而非 CMake？

| 特性 | WAF | CMake |
|------|-----|-------|
| 编写语言 | Python | CMakeLists DSL |
| 依赖跟踪 | 完美增量编译 | 一般 |
| 嵌入式支持 | 优秀 | 良好 |
| 学习曲线 | 平缓 | 陡峭 |
| 编译速度 | 极快（增量） | 快 |
| ArduPilot优化 | 深度集成 | 有第三方支持 |

**WAF优势：**
- ? Python编写，易于维护和扩展
- ? 完美的依赖跟踪，增量编译非常快
- ? 原生支持交叉编译和无操作系统编译
- ? 针对ArduPilot硬件编译深度优化

---

## 基础编译流程

### 1?? 编译前准备

```bash
# 克隆仓库（包含所有子模块）
git clone --recursive https://github.com/ArduPilot/ardupilot.git
cd ardupilot

# 查看支持的所有开发板
./waf list_boards
```

### 2?? 配置阶段 (Configure Phase)

在编译前，**必须先配置**，指定目标开发板和编译选项。

```bash
# 基本配置命令格式
./waf configure --board <board_name> [options]
```

#### 常用开发板配置

```bash
# Pixhawk 系列
./waf configure --board Pixhawk1          # Pixhawk 1
./waf configure --board CubeBlack         # Cube Black (Pixhawk 2.1)
./waf configure --board Pixracer          # Pixracer

# Linux 开发板
./waf configure --board navio2            # Navio 2
./waf configure --board navio             # Navio
./waf configure --board edge              # Emlid Edge

# 软件仿真
./waf configure --board sitl              # 软件仿真器
./waf configure --board sitl --debug      # 带调试符号的仿真器

# 其他常用
./waf configure --board skyviper-v2450    # SkyViper GPS 无人机
./waf configure --board bebop --static    # Bebop (需要静态链接)
```

#### 配置选项说明

| 选项 | 说明 | 示例 |
|------|------|------|
| `--board` | 指定目标开发板 | `--board CubeBlack` |
| `--debug` | 编译调试版本（保留符号表） | `./waf configure --board sitl --debug` |
| `--static` | 静态链接（某些嵌入式板需要） | `./waf configure --board bebop --static` |
| `--rsync-dest` | Linux板远程安装目标IP | `--rsync-dest root@192.168.1.2:/` |
| `--enable-benchmarks` | 启用基准测试编译 | `--enable-benchmarks` |

**?? 重要提示：**
- `configure` 只需运行一次，除非要更改配置
- 配置信息保存在 `.lock-waf_cygwin_build` 隐藏文件中

### 3?? 编译阶段 (Build Phase)

配置完成后，可以编译程序。

```bash
# 编译 ArduCopter（所有多旋翼类型）
./waf copter

# 其他车型
./waf plane          # 固定翼飞机（包括VTOL）
./waf heli           # 直升机
./waf rover          # 地面车和水面艇
./waf sub            # 潜水器/ROV
./waf antennatracker # 天线追踪器

# 通用编译（编译bin组的所有程序，这是默认行为）
./waf                # 等同于 ./waf --program-group bin
```

#### 并行编译加速

```bash
# 使用N个核心并行编译（大幅加速）
./waf copter -j8     # 使用8个核心

# waf 默认自动检测处理器数并并行编译
# 通常不需要显式指定 -j 选项，除非使用 icecc 加速编译
```

#### 编译特定目标

```bash
# 只编译 ArduCopter 二进制文件
./waf --targets bin/arducopter

# 编译单位测试
./waf --targets tests/test_math

# 编译所有可用目标
./waf list           # 先查看所有目标

# 编译多个特定目标
./waf --targets bin/arducopter bin/arduplane
```

#### 编译程序组

```bash
# 编译 bin 组（默认）- 包含所有主程序
./waf --program-group bin

# 编译 tests 组 - 单位测试
./waf --program-group tests

# 编译 examples 组 - 示例程序
./waf --program-group examples

# 编译 benchmarks 组 - 性能基准测试
./waf --program-group benchmarks

# 编译多个程序组
./waf --program-group tests --program-group benchmarks
```

### 4?? 输出二进制文件

编译完成后，二进制文件位置为：

```
build/<board-name>/bin/               # 主程序目录
build/<board-name>/tests/             # 测试程序目录
build/<board-name>/examples/          # 示例程序目录
build/<board-name>/benchmarks/        # 基准测试程序目录
```

#### 具体示例

```bash
# CubeBlack 编译结果
build/CubeBlack/bin/arducopter        # ArduCopter 固件

# SITL 编译结果
build/sitl/bin/arducopter             # 仿真器可执行文件
build/sitl/bin/arduplane

# Navio2 编译结果
build/navio2/bin/arducopter

# 测试程序
build/sitl/tests/test_math
build/sitl/tests/test_quaternion
```

---

## 完整编译示例

### 示例 1：编译 CubeBlack 版 ArduCopter

```bash
# 进入项目目录
cd ardupilot

# 配置编译环境
./waf configure --board CubeBlack

# 编译 ArduCopter（使用8核并行）
./waf copter -j8

# 编译完成
# 固件路径: build/CubeBlack/bin/arducopter
# 大小: ~1.5-2 MB（根据功能）
```

### 示例 2：编译 SITL 仿真器（带调试符号）

```bash
# 配置
./waf configure --board sitl --debug

# 编译
./waf copter -j8

# 运行仿真器
./build/sitl/bin/arducopter --help
./build/sitl/bin/arducopter -S               # 运行仿真器
```

### 示例 3：编译 Navio2 并远程安装

```bash
# 配置并指定远程安装目标
./waf configure --board navio2 --rsync-dest root@192.168.1.2:/

# 编译
./waf copter

# 编译并上传到开发板
./waf --targets bin/arducopter --upload
```

### 示例 4：只编译单位测试

```bash
# 配置
./waf configure --board sitl --enable-benchmarks

# 编译所有测试
./waf --program-group tests -j8

# 编译特定测试
./waf --targets tests/test_math

# 运行测试
./build/sitl/tests/test_math
```

---

## 清理构建

### 清理对象文件（保留配置）

```bash
# 只清理当前开发板的编译结果
./waf clean

# 编译后可以立即重新编译（增量编译会很快）
./waf copter
```

### 完全清理（删除所有）

```bash
# 删除所有开发板的编译结果和配置信息
./waf distclean

# 完全清理后需要重新 configure
./waf configure --board CubeBlack
```

### 同步子模块

```bash
# 重新同步子模块
./waf submodulesync

# 强制清理并重新同步子模块（解决某些问题）
./waf submodule_force_clean
```

---

## 上传/安装固件

### 上传到微控制器开发板

```bash
# 编译后直接上传到连接的 Pixhawk/Cube 开发板
./waf --targets bin/arducopter --upload

# 需要开发板通过 USB 连接到计算机
# WAF 会自动检测串口并上传
```

### 安装到 Linux 开发板

#### 远程上传（推荐）

```bash
# 配置时指定目标 IP 地址
./waf configure --board navio2 --rsync-dest root@192.168.1.2:/

# 编译并上传（使用 rsync）
./waf --targets bin/arducopter --upload
```

#### 本地安装（创建包）

```bash
# 编译程序
./waf copter

# 安装到临时目录
DESTDIR=/my/temp/location ./waf install

# 然后可以制作 .deb、.rpm 等包
# 或者手动复制文件到开发板
```

---

## 高级用法

### 查看所有编译信息

```bash
# 列出所有支持的开发板
./waf list_boards

# 列出所有可编译的目标
./waf list

# 列出所有 configure 选项
./waf -h configure

# 列出所有 build 选项
./waf -h build

# 查看完整帮助
./waf -h
```

### 使用不同编译器

#### 使用 Clang 替代 GCC

```bash
# 在 configure 时指定编译器
CXX=clang++ CC=clang ./waf configure --board=sitl

# 编译
./waf copter
```

#### 查看编译器信息

```bash
# 查看当前使用的编译器
./waf configure --board sitl
# 输出会显示使用的编译器版本
```

### 编译优化选项

```bash
# 编译发布版本（默认有优化）
./waf configure --board CubeBlack

# 编译调试版本（保留符号表，关闭优化）
./waf configure --board sitl --debug

# 静态链接（某些 Linux 发行版需要）
./waf configure --board bebop --static
```

### 编译特定功能

某些功能可以通过参数启用/禁用，编辑 `wscript` 文件或通过环境变量控制。

```bash
# 启用基准测试（需要在 configure 时指定）
./waf configure --board sitl --enable-benchmarks
./waf benchmarks -j8

# 查看是否有其他配置选项
./waf -h configure | grep -i enable
```

---

## 编译目录结构

编译后的目录结构如下：

```
ardupilot/
├── build/
│   ├── CubeBlack/
│   │   ├── bin/
│   │   │   ├── arducopter              # 多旋翼固件
│   │   │   ├── arduplane               # 固定翼固件
│   │   │   └── ...
│   │   ├── tests/
│   │   ├── examples/
│   │   └── .build_manifest
│   ├── sitl/
│   │   ├── bin/
│   │   │   ├── arducopter              # SITL 可执行文件
│   │   │   └── ...
│   │   └── tests/
│   │       ├── test_math
│   │       ├── test_quaternion
│   │       └── ...
│   └── [其他开发板]/
├── .lock-waf_cygwin_build              # 配置缓存
└── ...
```

---

## 常见问题与解决方案

### ? 找不到 `./waf` 命令

**问题：** 运行 `./waf` 提示找不到命令

**解决：**
```bash
# 1. 检查是否在 ardupilot 根目录
pwd                    # 应该显示 .../ardupilot
ls -la waf             # 应该能看到 waf 文件

# 2. 重新克隆仓库
git clone --recursive https://github.com/ArduPilot/ardupilot.git
cd ardupilot

# 3. 确保有执行权限
chmod +x waf
```

### ? 编译速度很慢

**问题：** 编译 ArduCopter 需要很长时间

**解决：**
```bash
# 1. 使用并行编译（最有效）
./waf copter -j8       # 根据 CPU 核心数调整

# 2. 已配置的情况下，第二次编译会快很多（增量编译）
./waf copter           # 第二次会极快

# 3. 检查磁盘速度
# 确保在 SSD 上编译，不要在网络共享上编译

# 4. 关闭不必要的后台程序
# 释放更多 CPU 和内存给编译器
```

### ? 权限不足

**问题：** 出现 "Permission denied" 错误

**解决：**
```bash
# ? 错误做法
sudo ./waf configure --board CubeBlack   # 不要用 sudo！

# ? 正确做法
./waf configure --board CubeBlack        # 直接运行，无需 sudo

# 如果有权限问题，检查文件权限
chmod +x waf
```

### ? 无法找到开发板

**问题：** `configure` 时找不到指定的开发板

**解决：**
```bash
# 1. 查看支持的开发板列表
./waf list_boards

# 2. 使用正确的开发板名称（大小写敏感）
./waf configure --board CubeBlack       # 正确
./waf configure --board cubeblack       # 可能错误

# 3. 检查是否需要特殊选项
./waf configure --board bebop --static  # bebop 需要 --static
```

### ? 编译错误：submodule 问题

**问题：** 编译时出现 submodule 相关错误

**解决：**
```bash
# 1. 重新同步子模块
./waf submodulesync

# 2. 强制重新初始化
./waf submodule_force_clean

# 3. 手动同步
git submodule update --init --recursive
```

### ? 如何编译其他车型？

**问题：** 只知道如何编译 Copter，不知道其他车型

**解决：**
```bash
# 查看所有程序组
./waf list

# 编译不同的车型
./waf plane              # 固定翼
./waf heli               # 直升机
./waf rover              # 地面车
./waf sub                # 潜水器
./waf antennatracker     # 天线追踪器

# 配置时不变，编译时改变目标
./waf configure --board CubeBlack
./waf copter             # 编译 Copter
./waf plane              # 编译 Plane
```

### ? 如何调试编译过程？

**问题：** 想了解编译过程或排查问题

**解决：**
```bash
# 1. 显示详细的编译信息
./waf copter --verbose

# 2. 只编译特定文件查看编译参数
./waf --targets bin/arducopter --verbose

# 3. 查看完整的编译命令
./waf copter -v           # 简短形式
./waf copter --verbose    # 完整形式
```

---

## 编译性能优化

### 使用 ccache 加速重复编译

```bash
# 1. 安装 ccache
sudo apt install ccache   # Ubuntu/Debian
brew install ccache       # macOS
choco install ccache      # Windows (Chocolatey)

# 2. 配置 WAF 使用 ccache
export CC="ccache gcc"
export CXX="ccache g++"
./waf configure --board sitl
./waf copter -j8

# 3. 查看 ccache 统计
ccache -s
```

### 使用 icecc（分布式编译）

```bash
# 1. 在多台计算机间共享编译任务
# 2. 配置方式类似 ccache
export CC="icecc gcc"
export CXX="iceccd g++"

# 3. 编译时使用更多线程
./waf copter -j16        # 可以超过 CPU 核心数
```

---

## 与开发工作流集成

### 快速迭代开发

```bash
# 1. 第一次编译（完整编译）
./waf configure --board sitl --debug
./waf copter -j8

# 2. 修改源代码后，重新编译（增量编译，极快）
./waf copter -j8

# 3. 如果改了参数定义，可能需要清理
./waf clean
./waf copter -j8
```

### 在 IDE 中使用 WAF

```bash
# Visual Studio Code
# 1. 创建任务 (tasks.json)
# 2. 配置命令为 ./waf configure --board ...
# 3. 配置命令为 ./waf copter -j8

# CLion / IntelliJ IDEA
# 1. File -> Project Structure -> Build Tools
# 2. 添加自定义 Build Tool: WAF
# 3. 在编译时调用 WAF 命令
```

---

## 总结

| 任务 | 命令 |
|------|------|
| 查看支持的开发板 | `./waf list_boards` |
| 配置编译环境 | `./waf configure --board <name>` |
| 编译 ArduCopter | `./waf copter -j8` |
| 编译其他车型 | `./waf plane\|heli\|rover\|sub` |
| 编译特定目标 | `./waf --targets bin/arducopter` |
| 清理构建 | `./waf clean` 或 `./waf distclean` |
| 上传固件 | `./waf --targets bin/arducopter --upload` |
| 查看所有目标 | `./waf list` |
| 查看帮助 | `./waf -h` 或 `./waf -h configure` |

---

**最后更新：** 2026年1月28日  
**适用版本：** ArduCopter 4.5.7
