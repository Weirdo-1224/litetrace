### 测试文档：`litetrace` 功能验证说明

#### 1. 测试环境与前置条件

- **操作系统**：Linux（内核已开启 ftrace 功能）
- **tracefs 挂载路径**（至少满足其一）：
  - `/sys/kernel/tracing`
  - `/sys/kernel/debug/tracing`
- **权限要求**：
  - 所有涉及访问 tracefs 控制文件的命令必须在 root 下执行：`sudo ./litetrace ...`
- **编译步骤**：

```bash
cd litetrace
make
# 期望：无 warning，生成 ./litetrace
```

---

#### 2. 基础检查

##### 2.1 帮助信息

```bash
./litetrace -h
./litetrace --help
```

**预期结果：**

- 退出码：`0`
- 输出包含：

```text
Usage:
  ./litetrace tracer function
  ./litetrace filter <func|pattern>
  ./litetrace filter-clear
  ./litetrace start
  ./litetrace stop
  ./litetrace status
  ./litetrace dump [-o file]
```

##### 2.2 无参数运行

```bash
./litetrace
```

**预期结果：**

- 退出码：`2`
- 输出包含：
  - `[ERR] missing argument`
  - 上述 usage 信息

##### 2.3 未知命令

```bash
./litetrace hello
```

**预期结果：**

- 退出码：`1`
- 输出包含：
  - `[ERR] unknown command`
  - usage 信息

---

#### 3. 权限相关测试

##### 3.1 非 root 运行 status

```bash
./litetrace status
```

**预期结果：**

- 退出码：`1`
- 输出包含：

```text
[ERR] permission denied, please run as root
```

##### 3.2 非 root 运行 tracer / start / dump

```bash
./litetrace tracer function
./litetrace start
./litetrace dump
```

**预期结果：**

- 均返回退出码 `1`
- 输出均包含：

```text
[ERR] permission denied, please run as root
```

---

#### 4. 功能测试（root 下）

以下命令均以 root 身份执行，例如：

```bash
sudo ./litetrace ...
```

##### 4.1 用例 1：查看默认状态

```bash
sudo ./litetrace status
```

**预期结果：**

- 退出码：`0`
- 输出固定三行：

```text
current_tracer     : <当前值>
tracing_on         : <0 或 1>
set_ftrace_filter  : <某个值或 <empty>>
```

当过滤器为空时第三行应为：

```text
set_ftrace_filter  : <empty>
```

##### 4.2 用例 2：设置 tracer 为 function

```bash
sudo ./litetrace tracer function
```

**预期结果：**

- 若系统支持 `function` tracer：
  - 退出码：`0`
  - 输出：

    ```text
    [OK] tracer set to function
    ```

  - 再执行：

    ```bash
    sudo ./litetrace status
    ```

    `current_tracer` 行应为：

    ```text
    current_tracer     : function
    ```

- 若系统不支持 `function`：
  - 退出码：`1`
  - 输出包含：

    ```text
    [ERR] tracer 'function' is not available on this system
    ```

##### 4.3 用例 3：非法 tracer 参数

```bash
sudo ./litetrace tracer abc
```

**预期结果：**

- 退出码：`1`
- 输出包含类似：

```text
[ERR] tracer 'abc' is not supported, only 'function' is allowed
```

---

#### 5. 过滤器相关测试

##### 5.1 用例 4：设置过滤器为指定函数

前置：已成功设置 `tracer function`。

```bash
sudo ./litetrace filter do_sys_open
```

**预期结果：**

- 退出码：`0`
- 输出：

```text
[OK] filter set to do_sys_open
```

- 再执行：

```bash
sudo ./litetrace status
```

第三行应为：

```text
set_ftrace_filter  : do_sys_open
```

##### 5.2 用例 5：设置过滤器为通配符

```bash
sudo ./litetrace filter vfs_*
```

**预期结果：**

- 退出码：`0`
- 输出：

```text
[OK] filter set to vfs_*
```

- `status` 中第三行应为：

```text
set_ftrace_filter  : vfs_*
```

##### 5.3 用例 6：缺少过滤器参数

```bash
sudo ./litetrace filter
```

**预期结果：**

- 退出码：`2`
- 输出包含：

```text
[ERR] missing argument
Usage:
  ./litetrace tracer function
  ...
```

##### 5.4 用例 7：清空过滤器

```bash
sudo ./litetrace filter-clear
```

**预期结果：**

- 退出码：`0`
- 输出：

```text
[OK] filter cleared
```

- 随后：

```bash
sudo ./litetrace status
```

第三行应恢复为：

```text
set_ftrace_filter  : <empty>
```

---

#### 6. tracing 开关测试

##### 6.1 用例 8：在非 function tracer 下 start（严格模式）

前置：把当前 tracer 改成非 function（可手动改 `current_tracer` 验证）。

执行：

```bash
sudo ./litetrace start
```

**预期结果：**

- 退出码：`1`
- 输出类似：

```text
[ERR] current_tracer is '<实际值>', please run 'litetrace tracer function' first
```

##### 6.2 用例 9：正常开启和关闭 tracing

前置：

```bash
sudo ./litetrace tracer function
sudo ./litetrace filter do_sys_open   # 可选
```

步骤：

```bash
sudo ./litetrace start
sudo ./litetrace status
sudo ./litetrace stop
sudo ./litetrace status
```

**预期结果：**

- `start`：
  - 退出码：`0`
  - 输出：

    ```text
    [OK] tracing started
    ```

- 第一次 `status` 中第二行：

  ```text
  tracing_on         : 1
  ```

- `stop`：
  - 退出码：`0`
  - 输出：

    ```text
    [OK] tracing stopped
    ```

- 第二次 `status` 中第二行：

  ```text
  tracing_on         : 0
  ```

---

#### 7. dump 功能测试

##### 7.1 用例 10：直接输出 trace 到终端

按 README 流程：

```bash
cd litetrace
sudo ./litetrace tracer function
sudo ./litetrace filter do_sys_open
sudo ./litetrace start

# 触发一些系统调用
ls
cat /etc/hostname

sudo ./litetrace stop
sudo ./litetrace dump
```

**预期结果：**

- `dump` 退出码：`0`
- 终端输出为 ftrace 标准 trace 内容，类似你给我的那段长输出（进程名 / CPU / 时间戳 / 函数调用链），**中间不带 `[OK]` 前缀**，仅在写文件模式才会有 `[OK]`。

##### 7.2 用例 11：导出到文件

```bash
sudo ./litetrace dump -o result.txt
```

**预期结果：**

- 退出码：`0`
- 终端输出：

```text
[OK] trace dumped to result.txt
```

- 文件检查：

```bash
ls result.txt
test -s result.txt && echo "non-empty"
```

  - `result.txt` 存在且非空
  - 打开 `result.txt` 能看到与 `dump` 直接输出类似的 trace 内容

##### 7.3 用例 12：`dump` 的非法参数

```bash
sudo ./litetrace dump -o
sudo ./litetrace dump --xxx
```

**预期结果：**

- 第一条：
  - 退出码：`2`
  - 输出包含 `[ERR] missing argument` 及 usage
- 第二条（未知选项）：
  - 退出码：`2`
  - 输出类似：

    ```text
    [ERR] unknown option '--xxx' for dump
    Usage:
      ./litetrace tracer function
      ...
    ```

---

#### 8. tracefs 不存在场景（可选）

在一个没有挂载 tracefs 的系统上（或暂时卸载 tracefs 后）执行：

```bash
./litetrace status
```

**预期结果：**

- 退出码：`3`
- 输出：

```text
[ERR] tracefs not found, ensure ftrace is enabled in kernel
```

---

#### 9. 快速回归脚本（示例）

可写一个简单的回归脚本方便反复测试（需 root）：

```bash
#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

echo "== build =="
make

echo "== help =="
./litetrace -h

echo "== basic status as root =="
sudo ./litetrace status || true

echo "== tracer function =="
sudo ./litetrace tracer function

echo "== filter and start =="
sudo ./litetrace filter do_sys_open
sudo ./litetrace start

echo "== trigger workload =="
ls >/dev/null
cat /etc/hostname >/dev/null

echo "== stop and dump =="
sudo ./litetrace stop
sudo ./litetrace dump -o result.txt

echo "== check result.txt =="
test -s result.txt && echo "[OK] result.txt non-empty" || echo "[ERR] result.txt empty"
```

运行：

```bash
chmod +x test_litetrace.sh
./test_litetrace.sh
```

---