# DreamUMG

`DreamUMG` 是一个面向 Unreal Engine UMG/Slate 的扩展插件，当前主要提供三类能力：

- 动态文本动画
- 2D UMG 中的伪 3D / 曲面变换
- 可由蓝图动态组装的 Dream Slate UI 树

当前版本基于 `UE 5.7` 开发和验证。

## 功能概览

### 1. Dream Animated Text

核心控件：

- `Dream Animated Text`

核心对象：

- `Typewriter Player`
- `Text Animation Player`
- `Range Selector`
- `Random Selector`
- `Lyrics Selector`
- 各类 `Executor`

目前内置的执行器包括：

- `Alpha Executor`
- `Color Executor`
- `Color Random Executor`
- `Position Executor`
- `Position Random Executor`
- `Position Wave Executor`
- `Rotation Random Executor`
- `Rotation Wave Executor`
- `Scale Executor`
- `Scale Random Executor`
- `Scale Wave Executor`

这套系统适合做：

- 打字机
- 字符渐显/渐隐
- 逐字位移、缩放、旋转
- 歌词/KTV 风格扫光高亮
- 随机扰动与波浪动画

### 2. 2D UMG 伪 3D 变换

核心控件：

- `Dream Pseudo 3D Box`
- `Dream Curved Box`
- `Dream Flip Card Box`

适合做：

- 普通 UMG 中的透视旋转
- 卡片翻转
- 面板倾斜
- 弯曲屏幕/圆柱屏效果

这两个控件本质上仍然运行在 2D Slate/UMG 渲染链里，不是世界空间 Widget，也不是字形网格级 3D。

### 3. Dream Slate UI 树

核心对象：

- `Dream Slate Host`
- `Dream Slate Widget Component`
- `Dream Widget Component`
- `Slate 3D Text`
- `Slate 3D Image`
- `Slate 3D Border`
- `Slate 3D Button`
- `Slate 3D Vertical Box`

这套对象可以在细节面板里直接嵌套，也可以在蓝图里运行时动态创建和拼装。

## 蓝图支持

`DreamUMG` 当前已经补齐了主要蓝图入口：

- 所有核心文本动画对象都支持 `BlueprintType / Blueprintable`
- 主要 Widget / Component 都支持蓝图访问
- 关键对象属性带 `BlueprintGetter / BlueprintSetter`
- 常用运行时结构操作已经提供蓝图函数

### 蓝图库

蓝图函数库：

- `DreamUMGBlueprintLibrary`

当前提供的创建函数：

- `CreateDreamTextAnimationPlayer`
- `CreateDreamTextAnimationSelector`
- `CreateDreamTextAnimationExecutor`
- `CreateDreamSlate3DWidget`
- `RefreshDreamSlate3DWidget`

### 文本动画运行时操作

`Text Animation Player` 目前支持在蓝图中：

- `SetSelector`
- `SetExecutors`
- `AddExecutor`
- `RemoveExecutor`
- `ClearExecutors`
- `Get Executors`
- `SetOffset`
- `ResetPlayerState`
- `NeedsTick`

`Selector` / `Executor` 目前支持：

- `SetOffset`
- `ResetSelectorState`
- `ResetExecutorState`
- `NeedsTick`

### Slate UI 树运行时操作

`Slate 3D` 节点当前支持：

- `RefreshOwner`
- `SetContentWidget`
- `AddChildWidget`
- `AddChildSlot`
- `RemoveChildWidget`
- `ClearChildren`

## 快速开始

### 1. 在 UMG 中使用文本动画

1. 在 Widget Blueprint 中放入 `Dream Animated Text`
2. 设置 `Text`
3. 给 `Animation Player` 指定一个对象

常见组合：

- 打字机：`Typewriter Player`
- 普通逐字动画：`Text Animation Player + Selector + Executors`

推荐入门配置：

- `Animation Player` 设为 `Text Animation Player`
- `Selector` 设为 `Range Selector`
- 添加一个 `Alpha Executor`
- 再添加一个 `Position Executor`

### 2. 在 UMG 中使用伪 3D

1. 放入 `Dream Pseudo 3D Box`
2. 把任意单个子控件放进去
3. 调整：

- `Rotation`
- `Perspective Strength`
- `Field Of View`
- `Depth Offset`
- `Perspective Segments`

如果想做圆柱/弯曲屏：

1. 放入 `Dream Curved Box`
2. 把任意单个子控件放进去
3. 调整：

- `Rotation`
- `Curve Angle`
- `Curve Segments`
- `Perspective Strength`
- `Depth Offset`
- `Transform Pivot`

如果想做真正的正反面翻牌：

1. 放入 `Dream Flip Card Box`
2. 在它下面放两个子控件
   - 第 1 个子控件是正面
   - 第 2 个子控件是背面
3. 用蓝图时间轴或动画驱动 `Flip Angle`

常用配置：

- `Flip Angle = 0` 显示正面
- `Flip Angle = 180` 显示背面
- `Flip Angle = 360` 回到正面
- `Mirror Back Face = true` 用于避免背面文字镜像
- `Horizontal Segments` 越高，透视翻转边缘越平滑

### 3. 在 UMG 中使用 Dream Slate Host

1. 放入 `Dream Slate Host`
2. 给 `Root Widget` 指定一个 `Dream Slate` 节点树

推荐结构：

- `Slate 3D Vertical Box`
- `Slate 3D Border`
- `Slate 3D Button`
- `Slate 3D Text`

### 4. 在蓝图中动态创建文本动画对象

示例流程：

1. `CreateDreamTextAnimationPlayer`
2. `CreateDreamTextAnimationSelector`
3. `CreateDreamTextAnimationExecutor`
4. `SetSelector`
5. `AddExecutor`
6. `SetAnimationPlayer`

### 5. 在蓝图中动态创建 Dream Slate UI 树

示例流程：

1. `CreateDreamSlate3DWidget` 创建 `Slate 3D Vertical Box`
2. 再创建 `Slate 3D Text / Button / Border`
3. 用 `AddChildWidget` 或 `SetContentWidget` 拼起来
4. 赋给 `Dream Slate Host` 或 `Dream Widget Component`
5. 调用 `RefreshDreamSlate3DWidget`

## 组件说明

### Dream Slate Widget Component

这是基于 `UWidgetComponent` 的 Dream Slate 承载组件，适合把 `Dream Slate` 树直接放进世界空间。

### Dream Widget Component

这是 Dream 的自定义 Widget 组件实现，目标是提供更独立的世界空间合成路径。

当前建议：

- 常规世界空间 UI：优先使用 `Dream Slate Widget Component`
- 需要 Dream 自定义渲染路径时：使用 `Dream Widget Component`

## 已知限制

### 1. Pseudo 3D / Curved Box 仍然是分段近似

这两套效果都依赖分段重投影：

- 分段越高，轮廓越平滑
- 分段越高，绘制成本也越高

### 2. 极端透视下会触发稳定性裁剪

为了避免大透视时出现飞刺、翻面和错误覆盖，目前对接近相机的分片做了保护：

- 过近分片可能被跳过
- 背面分片会被剔除
- 分片按深度排序绘制

这是有意为之，优先保证画面稳定。

### 3. 曲面与伪 3D 不是“真 3D”

它们不具备完整的：

- 场景深度遮挡
- 网格级几何
- 真实 3D 交互体积

如果目标是完整 3D UI，请考虑世界空间 Widget 或单独的 3D UI 渲染方案。

## 目录结构

```text
Plugins/DreamUMG
├─ Content
├─ Resources
├─ Shaders
└─ Source/DreamUMG
   ├─ Public
   │  ├─ Components
   │  └─ Widgets
   └─ Private
```

## 主要公开头文件

- `Public/Widgets/DreamAnimatedTextBlock.h`
- `Public/Widgets/DreamTextAnimation.h`
- `Public/Widgets/DreamPseudo3DBox.h`
- `Public/Widgets/DreamCurvedBox.h`
- `Public/Widgets/DreamFlipCardBox.h`
- `Public/Widgets/DreamSlate3DWidgets.h`
- `Public/Widgets/DreamSlateHostWidget.h`
- `Public/Components/DreamSlateWidgetComponent.h`
- `Public/Components/DreamWidgetComponent.h`
- `Public/DreamUMGBlueprintLibrary.h`

## 建议

如果你准备在项目里大规模使用：

- 文本动画优先走 `Dream Animated Text`
- 一般 UI 透视效果优先走 `Dream Pseudo 3D Box`
- 圆柱屏/弧面屏优先走 `Dream Curved Box`
- 蓝图动态拼 UI 时优先走 `Dream Slate Host + DreamUMGBlueprintLibrary`

如果你后面还要继续扩展，推荐下一步优先补：

- 更多 Blueprint setter/getter
- Slate 3D HorizontalBox / Overlay / UniformGrid
- Pseudo 3D / Curved Box 的 supersample 缓存
- 更多文本动画执行器
