# LiteTrace

一个轻量级的 Linux 内核函数追踪工具，基于 ftrace 实现。

## 功能特性

- 简单易用的命令行接口
- 支持内核函数调用追踪
- 灵活的函数过滤规则
- 追踪日志导出功能
- 完善的错误提示与权限检查

## 依赖要求

- Linux 内核（需启用 ftrace）
- GCC 编译器
- root 权限（用于访问 tracefs）

## 编译安装

```bash
make
sudo make install  # 可选，手动复制到 PATH 目录
```

清理编译产物：

```bash
make clean
```

## 使用方法

### 1. 设置追踪器

```bash
sudo ./litetrace tracer function
```

### 2. 配置过滤规则

追踪特定函数：
```bash
sudo ./litetrace filter schedule
```

支持通配符模式：
```bash
sudo ./litetrace filter 'tcp_*'
```

清空过滤器：
```bash
sudo ./litetrace filter-clear
```

### 3. 控制追踪

启动追踪：
```bash
sudo ./litetrace start
```

停止追踪：
```bash
sudo ./litetrace stop
```

### 4. 查看状态

```bash
sudo ./litetrace status
```

### 5. 导出日志

输出到控制台：
```bash
sudo ./litetrace dump
```

保存到文件：
```bash
sudo ./litetrace dump -o trace.log
```

## 完整示例

```bash
# 设置 function 追踪器
sudo ./litetrace tracer function

# 只追踪 schedule 相关函数
sudo ./litetrace filter schedule

# 开始追踪
sudo ./litetrace start

# ... 执行需要追踪的操作 ...

# 停止追踪
sudo ./litetrace stop

# 导出日志
sudo ./litetrace dump -o /tmp/trace.log

# 查看结果
cat /tmp/trace.log
```

## 项目结构

```
.
├── main.c           # 命令行入口
├── tracefs.c/h      # tracefs 上下文管理
├── tracer_ctrl.c/h  # 追踪器控制逻辑
├── file_util.c/h    # 文件操作工具
├── output.c/h       # 输出格式化
└── Makefile         # 构建配置
```

## 工作原理

LiteTrace 通过操作 Linux tracefs 文件系统接口实现追踪功能：

1. **自动检测** tracefs 挂载点（`/sys/kernel/tracing` 或 `/sys/kernel/debug/tracing`）
2. **设置追踪器** - 写入 `current_tracer` 文件
3. **配置过滤器** - 写入 `set_ftrace_filter` 文件
4. **控制开关** - 写入 `tracing_on` 文件
5. **读取日志** - 从 `trace` 文件导出数据

## 注意事项

- 必须以 root 权限运行（访问 tracefs 需要特权）
- 仅支持 `function` 追踪器
- 确保内核已启用 ftrace 功能
- 追踪会产生性能开销，生产环境谨慎使用

## 许可证

MIT License
