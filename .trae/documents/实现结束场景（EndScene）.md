# 实现结束场景（EndScene）

## 目标

按照链接要求，实现完整的游戏结束场景，处理游戏胜利和失败的情况，并提供重新开始和返回主菜单的选项。

## 实现步骤

### 1. 修改 SessionData 类

* 在 `session_data.h` 中添加 `is_win_` 字段，用于存储游戏胜利/失败状态

* 添加 `getIsWin()` 和 `setIsWin()` 方法

* 更新 `toJson()` 和 `fromJson()` 方法，支持序列化和反序列化 `is_win_` 字段

### 2. 创建 EndScene 类

* 创建 `end_scene.h` 和 `end_scene.cpp` 文件

* 实现 `EndScene` 类，继承自 `engine::scene::Scene`

* 实现 `createUI()` 方法，根据游戏结果显示不同的信息，如各个场景最高分，win还是game over，还有按钮back和restart，参考pause场景的示例创建

* 添加重新开始和返回主菜单的按钮

### 3. 修改 GameScene 类

* 添加游戏失败的触发逻辑，当玩家生命值为0时触发

* 修改游戏胜利的触发逻辑，使用 EndScene 而不是直接返回第一关

* 在游戏胜利或失败时，设置 SessionData 的 `is_win_` 字段

* 使用 `SceneManager::requestReplaceScene()` 切换到 EndScene

### 4. 更新 CMakeLists.txt

* 在 CMakeLists.txt 中添加 EndScene 的源文件

### 5. 测试和调试

* 测试游戏胜利的情况，确保 EndScene 正确显示

* 测试游戏失败的情况，确保 EndScene 正确显示

* 测试重新开始和返回主菜单的功能

## 技术要点

### EndScene 设计

* 根据 `SessionData::getIsWin()` 显示不同的信息

* 胜利时显示 "YOU WIN!"，失败时显示 "YOU DIED!"

* 显示当前得分和最高分

* 提供重新开始和返回主菜单的按钮

* m每次添加文件记住在cmakelist中添加

### 游戏流程

* 完整游戏循环：TitleScene → GameScene → EndScene → TitleScene

* 胜利或失败时保存游戏数据

* 重新开始时重置游戏数据

### 数据持久化

* 使用 SessionData 的 `save()` 方法保存游戏结果

* 使用 `load()` 方法加载游戏数据

## 预期效果

* 玩家到达终点时，显示胜利场景，包含得分和最高分

* 玩家生命值耗尽时，显示失败场景，包含得分和最高分

* 玩家可以选择重新开始或返回主菜单

* 游戏数据正确保存和加载

