下面这份可以直接贴给 Cursor 作为实现说明。内容按“项目目标 → 架构 → 模块 → 接口 → 数据结构 → 命令流程 → 错误处理 → 测试 → 开发约束”的顺序写，尽量让 Cursor 能按文档一步步生成代码。

------

# `litetrace` 详细模块设计说明书

## 1. 项目概述

### 1.1 项目名称

```
litetrace
```

### 1.2 项目目标

实现一个简易版 `trace-cmd` 工具，基于 Linux `ftrace` 提供以下能力：

1. 支持打开 `function` 跟踪
2. 支持设置 `function` 过滤器
3. 支持动态开启和关闭跟踪
4. 支持查看当前配置状态
   - `current_tracer`
   - `tracing_on`
   - `set_ftrace_filter`
5. 支持导出跟踪结果
   - 读取并打印 `trace` 文件中的内容
   - 可选导出到指定文件

### 1.3 实现方式

用户态程序，通过读写 `tracefs/debugfs` 下的控制文件完成配置与结果读取，不涉及内核代码修改。

### 1.4 推荐实现语言

C 语言

原因：

- 贴合 Linux 系统编程课程作业
- 便于体现文件操作、命令行解析、错误处理能力
- 依赖少，方便在实验环境中编译运行

------

## 2. 系统原理说明

Linux `ftrace` 通常暴露在以下路径之一：

- `/sys/kernel/tracing`
- `/sys/kernel/debug/tracing`

程序需要优先检测并使用存在的 tracing 根目录，然后通过以下控制文件实现功能：

| 文件名              | 作用                                 |
| ------------------- | ------------------------------------ |
| `current_tracer`    | 当前启用的 tracer 类型               |
| `tracing_on`        | tracing 开关，`1` 为开启，`0` 为关闭 |
| `set_ftrace_filter` | function tracer 的过滤函数列表       |
| `trace`             | 当前 trace buffer 内容               |
| `available_tracers` | 当前系统支持的 tracer 列表           |

本项目只聚焦 `function tracer`，不实现其他 tracer 类型。

------

## 3. 功能需求

## 3.1 必选功能

### 3.1.1 设置 tracer

支持设置当前 tracer 为 `function`

示例：

```bash
sudo ./litetrace tracer function
```

### 3.1.2 设置函数过滤器

支持设置函数过滤器，例如：

```bash
sudo ./litetrace filter do_sys_open
sudo ./litetrace filter vfs_*
```

### 3.1.3 动态启停 tracing

支持开启和关闭 tracing：

```bash
sudo ./litetrace start
sudo ./litetrace stop
```

### 3.1.4 查看当前状态

支持查看以下状态：

- `current_tracer`
- `tracing_on`
- `set_ftrace_filter`

示例：

```bash
sudo ./litetrace status
```

输出示例：

```text
current_tracer     : function
tracing_on         : 1
set_ftrace_filter  : do_sys_open
```

### 3.1.5 导出 trace 结果

支持读取 `trace` 的内容并打印，或导出到文件：

```bash
sudo ./litetrace dump
sudo ./litetrace dump -o trace.txt
```

------

## 3.2 可选增强功能

这些不是作业硬要求，但代码设计应预留扩展性：

- 清空过滤器：`filter-clear`
- 清空 trace buffer：`clear`
- 恢复默认配置：`reset`
- 列出支持的 tracer：`list`
- 提示是否必须 root 权限
- 自动检测是否存在 `function tracer`

------

## 4. 命令行设计

建议采用子命令风格。

## 4.1 支持的命令

### 4.1.1 设置 tracer

```bash
litetrace tracer function
```

### 4.1.2 设置过滤器

```bash
litetrace filter <function_name_or_pattern>
```

示例：

```bash
litetrace filter do_sys_open
litetrace filter vfs_*
```

### 4.1.3 清空过滤器

```bash
litetrace filter-clear
```

### 4.1.4 开启 tracing

```bash
litetrace start
```

### 4.1.5 关闭 tracing

```bash
litetrace stop
```

### 4.1.6 查看状态

```bash
litetrace status
```

### 4.1.7 导出 trace

```bash
litetrace dump
litetrace dump -o trace.txt
```

### 4.1.8 帮助信息

```bash
litetrace -h
litetrace --help
```

------

## 5. 总体架构设计

建议采用分层模块设计，避免把所有逻辑塞进 `main.c`。

### 5.1 模块划分

```text
litetrace/
├── main.c               // 程序入口，参数解析与命令分发
├── tracefs.h            // tracefs 路径与上下文定义
├── tracefs.c            // tracefs 根目录探测、路径初始化
├── file_util.h          // 文件读写基础接口
├── file_util.c          // 通用文件读写逻辑
├── tracer_ctrl.h        // tracer 控制接口
├── tracer_ctrl.c        // start/stop/tracer/filter/status/dump 实现
├── output.h             // 输出与错误信息封装
├── output.c             // 格式化输出
├── Makefile             // 编译脚本
└── README.md            // 使用说明
```

### 5.2 分层说明

#### 第 1 层：入口层

```
main.c
```

- 负责解析命令行参数
- 调用具体业务模块
- 不做复杂文件读写逻辑

#### 第 2 层：业务控制层

```
tracer_ctrl.c
```

- 实现 tracer 设置
- 实现 filter 设置
- 实现 start / stop
- 实现 status / dump

#### 第 3 层：基础支撑层

```
tracefs.c` + `file_util.c
```

- 检测 tracefs 路径
- 提供统一文件读写能力
- 屏蔽底层系统调用细节

#### 第 4 层：输出层

```
output.c
```

- 统一成功、错误、状态信息输出格式

------

## 6. 模块详细设计

# 6.1 `tracefs` 模块

## 6.1.1 模块职责

负责：

- 自动检测 tracing 根目录
- 构造所有控制文件的完整路径
- 为其他模块提供统一上下文

## 6.1.2 支持的根路径

按如下优先级探测：

1. `/sys/kernel/tracing`
2. `/sys/kernel/debug/tracing`

如果两个都不存在，返回错误。

## 6.1.3 数据结构设计

```c
#ifndef TRACEFS_H
#define TRACEFS_H

#include <limits.h>

typedef struct tracefs_ctx {
    char root[PATH_MAX];
    char current_tracer[PATH_MAX];
    char tracing_on[PATH_MAX];
    char set_ftrace_filter[PATH_MAX];
    char trace[PATH_MAX];
    char available_tracers[PATH_MAX];
} tracefs_ctx_t;

int tracefs_init(tracefs_ctx_t *ctx);

#endif
```

## 6.1.4 对外接口

### `int tracefs_init(tracefs_ctx_t *ctx);`

功能：

- 检测 tracing 根目录
- 初始化所有控制文件路径

输入：

- `ctx`：输出上下文

输出：

- 成功返回 `0`
- 失败返回 `-1`

## 6.1.5 实现要点

- 使用 `access(path, F_OK)` 检测目录是否存在
- 使用 `snprintf` 拼接路径
- 初始化前先清空结构体内容

## 6.1.6 错误场景

- tracing 文件系统不存在
- 路径拼接失败（理论上极少，但代码要检查）

------

# 6.2 `file_util` 模块

## 6.2.1 模块职责

提供统一的文本文件读取、写入能力，用于访问 tracefs 控制文件。

## 6.2.2 对外接口设计

```c
#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stddef.h>

int read_text_file(const char *path, char *buf, size_t size);
int write_text_file(const char *path, const char *text);
int write_empty_file(const char *path);

#endif
```

## 6.2.3 接口说明

### `int read_text_file(const char *path, char *buf, size_t size);`

功能：

- 读取文本文件内容到缓冲区
- 自动补 `\0`
- 可选去掉末尾多余换行

返回：

- 成功返回读取字节数
- 失败返回 `-1`

### `int write_text_file(const char *path, const char *text);`

功能：

- 将字符串写入指定文件
- 用于写 `current_tracer`、`tracing_on`、`set_ftrace_filter`

返回：

- 成功返回 `0`
- 失败返回 `-1`

### `int write_empty_file(const char *path);`

功能：

- 写空内容到文件
- 用于清空 `set_ftrace_filter`
- 也可用于清空 `trace`

返回：

- 成功返回 `0`
- 失败返回 `-1`

## 6.2.4 实现建议

- 使用 `open/read/write/close`
- 或使用 `fopen/fgets/fputs`
- 推荐系统调用风格，更贴近系统编程

## 6.2.5 关键细节

- `read` 后手动补 `\0`
- 避免缓冲区溢出
- 对 `write` 的返回值进行校验，防止部分写入
- 统一封装 errno 处理

## 6.2.6 辅助函数建议

可以增加一个内部函数：

```c
static void trim_trailing_newline(char *s);
```

作用：

- 把读出来的内容末尾的 `\n` 去掉，便于 status 输出

------

# 6.3 `tracer_ctrl` 模块

## 6.3.1 模块职责

封装所有跟踪控制逻辑，是项目的核心模块。

## 6.3.2 对外接口设计

```c
#ifndef TRACER_CTRL_H
#define TRACER_CTRL_H

#include "tracefs.h"

int lt_set_tracer(tracefs_ctx_t *ctx, const char *tracer);
int lt_set_filter(tracefs_ctx_t *ctx, const char *filter);
int lt_clear_filter(tracefs_ctx_t *ctx);
int lt_start(tracefs_ctx_t *ctx);
int lt_stop(tracefs_ctx_t *ctx);
int lt_status(tracefs_ctx_t *ctx);
int lt_dump(tracefs_ctx_t *ctx, const char *outfile);

#endif
```

------

## 6.3.3 子功能设计

### A. `lt_set_tracer`

#### 原型

```c
int lt_set_tracer(tracefs_ctx_t *ctx, const char *tracer);
```

#### 功能

设置当前 tracer。

本作业只要求支持：

- `function`

#### 处理流程

1. 检查 `tracer` 是否为 `"function"`
2. 读取 `available_tracers`
3. 检查其中是否包含 `function`
4. 如果支持，则写入 `current_tracer`
5. 输出成功信息

#### 失败场景

- tracer 参数为空
- tracer 不是 `function`
- 系统不支持 `function`
- 权限不足导致写失败

#### 设计要求

- 不要硬编码“系统一定支持 function”
- 必须对 `available_tracers` 做检查

------

### B. `lt_set_filter`

#### 原型

```c
int lt_set_filter(tracefs_ctx_t *ctx, const char *filter);
```

#### 功能

设置函数过滤器。

#### 处理流程

1. 检查 `filter` 是否为空
2. 直接写入 `set_ftrace_filter`
3. 若写失败，提示函数名或模式无效

#### 支持形式

- 单个函数名：`do_sys_open`
- 通配符：`vfs_*`

#### 注意点

不同内核对于非法函数名的报错形式不同，程序只需检测写入返回值并给出统一提示。

------

### C. `lt_clear_filter`

#### 原型

```c
int lt_clear_filter(tracefs_ctx_t *ctx);
```

#### 功能

清空当前过滤器。

#### 处理流程

- 向 `set_ftrace_filter` 写空字符串

------

### D. `lt_start`

#### 原型

```c
int lt_start(tracefs_ctx_t *ctx);
```

#### 功能

开启 tracing。

#### 处理流程

1. 可选检查 `current_tracer` 是否已为 `function`
2. 向 `tracing_on` 写入 `"1"`

#### 说明

若未设置 `function tracer`，可以：

- 严格模式：返回错误并提示先执行 `tracer function`
- 宽松模式：允许直接开启

建议使用严格模式，便于作业展示逻辑清晰。

------

### E. `lt_stop`

#### 原型

```c
int lt_stop(tracefs_ctx_t *ctx);
```

#### 功能

关闭 tracing。

#### 处理流程

- 向 `tracing_on` 写入 `"0"`

------

### F. `lt_status`

#### 原型

```c
int lt_status(tracefs_ctx_t *ctx);
```

#### 功能

输出当前配置状态。

#### 处理流程

1. 读取 `current_tracer`
2. 读取 `tracing_on`
3. 读取 `set_ftrace_filter`
4. 格式化打印

#### 输出要求

固定格式，便于老师检查：

```text
current_tracer     : function
tracing_on         : 1
set_ftrace_filter  : do_sys_open
```

若过滤器为空：

```text
set_ftrace_filter  : <empty>
```

------

### G. `lt_dump`

#### 原型

```c
int lt_dump(tracefs_ctx_t *ctx, const char *outfile);
```

#### 功能

导出 trace 内容。

#### 处理流程

1. 读取 `trace`
2. 若 `outfile == NULL`
   - 打印到标准输出
3. 否则
   - 写入指定文件

#### 可选增强

- 导出前提示用户先 `stop`
- 或由程序内部先 `stop` 再导出

为了避免副作用，建议默认不自动 stop，只导出当前内容。

#### 注意

`trace` 可能比较大，不建议一次只用很小缓冲区读取。可设计为：

- 先循环读取再输出
- 或设置一个较大的读取缓冲区

------

# 6.4 `output` 模块

## 6.4.1 模块职责

统一管理程序输出风格，保证提示一致。

## 6.4.2 对外接口建议

```c
#ifndef OUTPUT_H
#define OUTPUT_H

void print_ok(const char *fmt, ...);
void print_err(const char *fmt, ...);
void print_info(const char *fmt, ...);
void print_usage(const char *prog);

#endif
```

## 6.4.3 输出风格

### 成功

```text
[OK] tracer set to function
[OK] filter set to do_sys_open
[OK] tracing started
```

### 错误

```text
[ERR] tracefs not found
[ERR] permission denied
[ERR] invalid filter function or pattern
```

### 用法

```text
Usage:
  litetrace tracer function
  litetrace filter <func|pattern>
  litetrace filter-clear
  litetrace start
  litetrace stop
  litetrace status
  litetrace dump [-o file]
```

------

# 6.5 `main` 模块

## 6.5.1 模块职责

- 解析命令行参数
- 初始化 `tracefs_ctx_t`
- 命令分发到业务函数
- 返回程序退出码

## 6.5.2 主流程

1. 检查参数数量
2. 处理 `-h/--help`
3. 调用 `tracefs_init`
4. 根据子命令调用业务函数
5. 根据返回值输出退出状态

## 6.5.3 分发逻辑建议

伪代码：

```c
int main(int argc, char *argv[]) {
    tracefs_ctx_t ctx;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    if (tracefs_init(&ctx) != 0) {
        print_err("tracefs not found");
        return 1;
    }

    if (strcmp(argv[1], "tracer") == 0) {
        // argc check
        // lt_set_tracer(...)
    } else if (strcmp(argv[1], "filter") == 0) {
        // lt_set_filter(...)
    } else if (strcmp(argv[1], "filter-clear") == 0) {
        // lt_clear_filter(...)
    } else if (strcmp(argv[1], "start") == 0) {
        // lt_start(...)
    } else if (strcmp(argv[1], "stop") == 0) {
        // lt_stop(...)
    } else if (strcmp(argv[1], "status") == 0) {
        // lt_status(...)
    } else if (strcmp(argv[1], "dump") == 0) {
        // parse -o
        // lt_dump(...)
    } else {
        print_err("unknown command");
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
```

------

## 7. 数据结构设计

## 7.1 `tracefs_ctx_t`

用于保存所有 tracefs 控制文件路径。

```c
typedef struct tracefs_ctx {
    char root[PATH_MAX];
    char current_tracer[PATH_MAX];
    char tracing_on[PATH_MAX];
    char set_ftrace_filter[PATH_MAX];
    char trace[PATH_MAX];
    char available_tracers[PATH_MAX];
} tracefs_ctx_t;
```

## 7.2 状态读取缓冲区

建议定义统一常量：

```c
#define LT_SMALL_BUF 256
#define LT_MEDIUM_BUF 1024
#define LT_LARGE_BUF 8192
```

用途：

- `current_tracer` / `tracing_on` 用小缓冲区
- `available_tracers` / `set_ftrace_filter` 用中缓冲区
- `trace` 导出用大缓冲区循环读取

------

## 8. 错误处理设计

## 8.1 统一原则

- 所有系统调用失败都必须检查返回值
- 所有用户输入错误都必须给出清晰提示
- 所有控制文件访问失败都要说明可能原因

## 8.2 常见错误场景

### 8.2.1 tracefs 不存在

提示：

```text
[ERR] tracefs not found, ensure ftrace is enabled in kernel
```

### 8.2.2 权限不足

提示：

```text
[ERR] permission denied, please run as root
```

### 8.2.3 tracer 不支持

提示：

```text
[ERR] tracer 'function' is not available on this system
```

### 8.2.4 filter 非法

提示：

```text
[ERR] invalid filter function or pattern
```

### 8.2.5 参数错误

提示：

```text
[ERR] missing argument
```

并打印 usage。

------

## 8.3 返回码规范

建议统一：

- `0`：成功
- `1`：一般错误
- `2`：参数错误
- `3`：环境错误（如 tracefs 不存在）
- `4`：权限错误

Cursor 实现时可简化为 `0/1`，但最好预留明确语义。

------

## 9. 关键流程设计

# 9.1 `tracer function` 流程

```text
main
  -> tracefs_init
  -> lt_set_tracer
       -> read available_tracers
       -> check "function"
       -> write current_tracer = "function"
       -> print ok
```

# 9.2 `filter xxx` 流程

```text
main
  -> tracefs_init
  -> lt_set_filter
       -> write set_ftrace_filter = <xxx>
       -> print ok
```

# 9.3 `start` 流程

```text
main
  -> tracefs_init
  -> lt_start
       -> optional read current_tracer
       -> write tracing_on = "1"
       -> print ok
```

# 9.4 `status` 流程

```text
main
  -> tracefs_init
  -> lt_status
       -> read current_tracer
       -> read tracing_on
       -> read set_ftrace_filter
       -> print formatted status
```

# 9.5 `dump` 流程

```text
main
  -> tracefs_init
  -> lt_dump
       -> read trace
       -> print to stdout or write to file
```

------

## 10. 状态与文件映射关系

| 用户命令          | 对应操作文件        | 写入/读取内容   |
| ----------------- | ------------------- | --------------- |
| `tracer function` | `current_tracer`    | 写入 `function` |
| `start`           | `tracing_on`        | 写入 `1`        |
| `stop`            | `tracing_on`        | 写入 `0`        |
| `filter xxx`      | `set_ftrace_filter` | 写入函数名/模式 |
| `filter-clear`    | `set_ftrace_filter` | 写入空串        |
| `status`          | `current_tracer`    | 读取            |
| `status`          | `tracing_on`        | 读取            |
| `status`          | `set_ftrace_filter` | 读取            |
| `dump`            | `trace`             | 读取            |

------

## 11. 编码规范要求

这部分很适合给 Cursor，能减少它生成混乱代码。

## 11.1 通用要求

1. 所有 `.c` 文件必须包含对应 `.h`
2. 所有对外函数必须在 `.h` 中声明
3. 内部辅助函数使用 `static`
4. 严格检查每个系统调用返回值
5. 使用 `snprintf` 替代 `sprintf`
6. 禁止魔法数字，统一用宏定义
7. 禁止把所有功能写进 `main.c`

## 11.2 命名规范

- 结构体：`xxx_t`
- 模块函数：`lt_` 或模块名前缀
- 局部辅助函数：`static int do_xxx(...)`

示例：

- `tracefs_init`
- `read_text_file`
- `lt_set_tracer`
- `lt_dump`

## 11.3 头文件保护

每个头文件必须使用 include guard：

```c
#ifndef TRACEFS_H
#define TRACEFS_H
...
#endif
```

## 11.4 注释要求

每个对外函数前写简短注释，说明：

- 功能
- 参数
- 返回值

------

## 12. Makefile 设计要求

建议生成目标文件和可执行文件：

```make
CC = gcc
CFLAGS = -Wall -Wextra -O2

OBJS = main.o tracefs.o file_util.o tracer_ctrl.o output.o

litetrace: $(OBJS)
	$(CC) $(CFLAGS) -o litetrace $(OBJS)

main.o: main.c tracefs.h tracer_ctrl.h output.h
tracefs.o: tracefs.c tracefs.h
file_util.o: file_util.c file_util.h
tracer_ctrl.o: tracer_ctrl.c tracer_ctrl.h tracefs.h file_util.h output.h
output.o: output.c output.h

clean:
	rm -f *.o litetrace
```

要求：

- 打开 `-Wall -Wextra`
- 编译无 warning
- 支持 `make` 和 `make clean`

------

## 13. 测试设计

# 13.1 功能测试用例

## 用例 1：查看默认状态

命令：

```bash
sudo ./litetrace status
```

预期：

- 能显示三项状态
- 无崩溃

## 用例 2：设置 tracer

命令：

```bash
sudo ./litetrace tracer function
```

预期：

- 显示成功信息
- `status` 中 `current_tracer` 为 `function`

## 用例 3：设置过滤器

命令：

```bash
sudo ./litetrace filter do_sys_open
```

预期：

- 显示成功信息
- `status` 中过滤器内容正确

## 用例 4：启动和停止 tracing

命令：

```bash
sudo ./litetrace start
sudo ./litetrace stop
```

预期：

- `status` 中 `tracing_on` 对应变化为 `1` 和 `0`

## 用例 5：导出 trace

流程：

```bash
sudo ./litetrace tracer function
sudo ./litetrace filter do_sys_open
sudo ./litetrace start
ls
cat /etc/hostname
sudo ./litetrace stop
sudo ./litetrace dump
```

预期：

- 输出 trace 内容
- 包含相关函数调用记录

## 用例 6：导出到文件

命令：

```bash
sudo ./litetrace dump -o result.txt
```

预期：

- 生成 `result.txt`
- 内容非空

------

# 13.2 异常测试用例

## 用例 7：非 root 运行

命令：

```bash
./litetrace start
```

预期：

- 提示权限不足

## 用例 8：非法 tracer

命令：

```bash
sudo ./litetrace tracer abc
```

预期：

- 提示仅支持 `function`

## 用例 9：缺少 filter 参数

命令：

```bash
sudo ./litetrace filter
```

预期：

- 提示参数缺失
- 打印 usage

## 用例 10：未知命令

命令：

```bash
sudo ./litetrace hello
```

预期：

- 提示 unknown command

------

## 14. 建议开发顺序

为了让 Cursor 逐步生成、逐步验证，建议按下面顺序实现：

### 第一步：基础框架

先生成：

- `tracefs.h/.c`
- `file_util.h/.c`
- `output.h/.c`
- `main.c` 基本框架
- `Makefile`

目标：

- 能成功编译
- `litetrace status` 至少能跑通初始化

### 第二步：实现状态查询

优先完成：

- `tracefs_init`
- `read_text_file`
- `lt_status`

因为最容易测试，也最能验证路径和文件读写正确性。

### 第三步：实现控制逻辑

再实现：

- `lt_set_tracer`
- `lt_set_filter`
- `lt_clear_filter`
- `lt_start`
- `lt_stop`

### 第四步：实现导出逻辑

最后实现：

- `lt_dump`
- `dump -o file`

### 第五步：完善错误处理和帮助信息

补充：

- 统一错误码
- usage
- 非法参数检查
- root 权限提示

------

## 15. 给 Cursor 的明确实现要求

下面这段你可以直接贴给 Cursor 当任务指令。

------

请用 C 语言实现一个命令行工具 `litetrace`，要求如下：

1. 项目采用模块化设计，不要把所有代码写在一个文件里。
2. 至少拆分为以下文件：
   - `main.c`
   - `tracefs.h` / `tracefs.c`
   - `file_util.h` / `file_util.c`
   - `tracer_ctrl.h` / `tracer_ctrl.c`
   - `output.h` / `output.c`
   - `Makefile`
3. 功能必须支持：
   - `litetrace tracer function`
   - `litetrace filter <func|pattern>`
   - `litetrace filter-clear`
   - `litetrace start`
   - `litetrace stop`
   - `litetrace status`
   - `litetrace dump`
   - `litetrace dump -o <file>`
4. tracing 根目录优先使用 `/sys/kernel/tracing`，不存在则尝试 `/sys/kernel/debug/tracing`
5. 需要操作的文件包括：
   - `current_tracer`
   - `tracing_on`
   - `set_ftrace_filter`
   - `trace`
   - `available_tracers`
6. `tracer` 命令只允许 `function`
7. 设置 tracer 前必须检查 `available_tracers` 是否支持 `function`
8. 所有系统调用都必须检查返回值
9. 所有头文件必须有 include guard
10. 编译选项使用 `-Wall -Wextra -O2`
11. 代码尽量清晰、简洁、可读，不要引入多余依赖
12. 输出格式统一：

- 成功：`[OK] ...`
- 失败：`[ERR] ...`

1. `status` 输出固定三行：

- `current_tracer`
- `tracing_on`
- `set_ftrace_filter`

1. `dump` 支持输出到标准输出或写入指定文件
2. 请同时生成 `README.md`，包含编译与使用示例

------

## 16. 交付标准

最终代码应满足：

1. `make` 能成功编译
2. 无明显 warning
3. `status` 能读取并显示当前配置
4. `tracer function`、`filter`、`start/stop` 能正常工作
5. `dump` 能打印或导出 trace 内容
6. 异常输入有清晰提示
7. 代码结构清晰，适合作业提交

