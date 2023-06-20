# UE5RuntimeTransformGizmo

# English Documentation
[English Documentation](README_EN.md)

## 更新说明
用于运行时的简单交互式变换小工具~  
初步实现了触控操作~  
![](Documentation/Images/visual_translate.png)  
![](Documentation/Images/visual_rotate.png)  
![](Documentation/Images/visual_scale.png)  
![](Documentation/Images/visual_combined.png)  


## 使用方法
1. 打开该项目并在内容浏览器中右击Mytaverse这个文件夹，选择迁移到你的项目Content目录下  
![](Documentation/Images/migrate_content.png)  
    - 当然也可以直接跳过打开这个项目，直接在Window的文件夹浏览器中将项目Content文件夹下的Mytaverse文件夹整个复制到你的项目的Content文件夹下

2. 打开你的项目所使用的Pawn（或Character）蓝图类（我这个功能是纯蓝图做的，所以只能在蓝图里使用），添加一个名为Pointer Interaction的组件  
![](Documentation/Images/add_component.gif)

3. 在Pawn蓝图的BeginPlay事件逻辑后面调用PointerInteraction组件的SetupInteraction节点，同时调用PlayerController的SetShowMouseCursor来显示鼠标  
![](Documentation/Images/pawn_beginplay.png)
    - InteractionSource 参数要设为MouseCursor，启用鼠标交互
    - InteractionDistance 表示鼠标交互距离，一般设置一个比较大的距离
    - UseMultiRayHit 这个必须启用
    - InteractionTypes 要设置为WorldDynamic
    - Activate 这个地方勾选才能启用交互功能

4. 在Pawn蓝图中添加LeftMouseButton事件并接入PointerInteraction组件的PressPointer和ReleasePointer方法
![](Documentation/Images/mouse_click.png)
    - 如果鼠标悬停在TransformGizmo上时，按下鼠标左键后会进入拖拽状态，这个时候我们一般不希望视角发生变化。  
    我这里判断PointerInteraction进入拖拽状态之后，就调用PlayerController的SetIgnoreLookInput来禁用视角控制，在鼠标松开之后在启用视角控制。

5. 在需要显示并启用Gizmo的地方，只需要对BP_TransformGizmo对象调用ActivateGizmo就可以了，当然需要先在场景里放置或生成一个BP_TransformGizmo对象  
![](Documentation/Images/activate_gizmo.png)  
    - ActivateGizmo方法的InitialTransform表示显示Gizmo时候的初始Transform，一般在对一个Actor启用Gizmo的时候要把该Actor的Transform传入
    - TransformEvent会在拖拽Gizmo的时候持续调用并传出当前计算得到的Transform结果，这个结果可以根据需要来使用

6. 控制函数列表
![](Documentation/Images/control_api.png)

## 其他
- UE5其实已经内置这个功能了，可以运行时使用，可以在引擎的[InteractiveToolsFramework](https://github.com/EpicGames/UnrealEngine/tree/release/Engine/Source/Runtime/InteractiveToolsFramework)模块中找到相关代码  
![](https://images.squarespace-cdn.com/content/v1/574f72911d07c08c97939643/1608324621401-5CLYGEIS5S5GF1CNNS0M/ToolsFrameworkDemo_Gizmo.png?format=300w)  
[参考文章1](https://www.gradientspace.com/tutorials/2022/6/1/the-interactive-tools-framework-in-ue5)  
[参考文章2](https://www.gradientspace.com/tutorials/2021/01/19/the-interactive-tools-framework-in-ue426)
- 可以尝试一下[ScriptableToolsFramework](https://github.com/EpicGames/UnrealEngine/tree/407acc04a93f09ecb42c07c98b74fd00cc967100/Engine/Plugins/Experimental/ScriptableToolsFramework/Source/ScriptableToolsFramework)模块

