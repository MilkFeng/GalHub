## 简介

存档收集小工具，可以将散落在各处的 GalGame 存档收集在一个文件夹中

使用 MinHook 和 DLL 注入来实现路径的重定向

目前还在开发中

## 如何使用

未完待续。。。

## 如何从源码构建

### 配置 Qt6 环境

最好使用静态链接 Qt

1. 安装 Qt6-MSVC，如果想要静态链接 Qt，则需自行编译 Qt（或者下载别人构建好的包，例如 https://build-qt.fsu0413.me）
2. 找到类似 C:\Qt\6.5.3\Qt6.5.3-Windows-x86_64-VS2022-17.10.0-staticFull 这样的目录
3. 将 build.bat 中的 QT6_PATH 变量设置成找到的目录

### 配置 Visual Studio 环境

Visual Studio 版本最好与前面 Qt 的版本一致

1. 安装对应版本的 Build Tools：打开 https://learn.microsoft.com/en-us/visualstudio/releases/2022/release-history 页面，下载对应版本的 Build Tools 安装工具
2. 打开安装工具，安装“使用 C++ 的桌面开发”相关的组件
3. 找到类似 C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat 的文件
4. 将 build.bat 中的 VC_VARS_BAT 变量设置成找到的文件

### 构建

打开根目录，执行 build.bat 即可

```
./build.bat
```

构建脚本可以有三个开关：debug、clean、run，如果想打开对应的开关，将它作为参数传给 build.bat 即可

### 安装

构建完后，会将构建产物放到 dist 文件夹下，构建产物有：

- GalHub.exe
- Bin/Hook-x86.dll
- Bin/Hook-x64.dll
- Bin/Helper.exe

## 感谢

- https://github.com/SegaraRai/PathRedirector 中使用 MinHook 对 ntdll 中的文件方法进行 Hook
- https://github.com/SegaraRai/InjectExec 中实现了 DLL 注入
