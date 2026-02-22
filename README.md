# 基于八叉树的裁剪网格生成

## 题目介绍

裁剪网格是从体素或隐式场中裁剪出一个等值面（等值曲面），并将该曲面转化为可渲染的三角网格。对于隐式函数 $f(x,y,z)$，我们通过方程 $f(x,y,z)=\text{isovalue}$ 定义目标曲面，再在体素网格中提取该等值面。传统 Marching Cubes（MC）通过 256 种体元配置表生成三角片；在高分辨率体素网格下，平坦区域容易产生大量重复小三角形，导致面片数量与内存开销显著上升。

本实现采用“八叉树组织 + MC 提取”的流程：先仅保留边界体元并插入八叉树，再通过 shrink 条件在树上进行合并，最后在叶节点覆盖范围内逐体元执行 MC。这样既能利用树结构减少无效遍历，又保持提取阶段的稳定性和一致性。

当前代码的关键点包括：

- 体素扫描阶段仅保留边界体元（`config != 0 && config != 255`）；
- 使用共面类型（`NormalTypeId`）和参数 `D` 作为 shrink 合并判据；
- 通过 `remap_cfg_to_mc` 统一角点编号到标准 MC 顺序；
- 交点采用线性插值，法向通过梯度校正，面片加入前进行退化与边约束检查。

## 算法流程

1. 体素扫描与边界体元筛选
   - 角点状态缓存到 `m_pointState`（`1=inside`, `0=outside`）。
  - 按代码中的角点索引顺序构造 `config`，仅保留边界体元。
   - 边界体元通过 `create_to_leaf(x,y,z)` 插入八叉树。

2. 共面参数计算与节点标记
   - 对每个边界叶子存储 `parms.config` 与 `parms.d`。
   - `d` 由 `kConfigToEqType` 与 `kEqTypeToEqQuad` 查表计算。

3. Shrink 合并
   - 使用队列自底向上检查父节点。
   - 若子节点 `NormalTypeId` 一致且 `D` 一致，则合并到父节点并释放子节点。
  - 该步骤用于压缩树结构并减少后续遍历开销。

4. 连通性相关的点状态重采样
   - `refine_point_state()` 对边界体元角点重新按隐式函数判定 inside/outside。
   - 当前实现未启用邻域多数投票平滑（日志中标注 `smoothing disabled`）。

5. 三角片提取
   - BFS 遍历八叉树叶节点；对每个叶节点覆盖的 `(x,y,z)` 体元逐个计算 `cell_config`。
   - 使用 `remap_cfg_to_mc` 转为标准 MC 顶点编号，再通过 `kTriTable` 生成三角形。
   - 边交点由 `intersect_edge` 线性插值获得。

6. 网格质量约束
   - 利用三角形中心梯度 `gradient(triCenter)` 统一法向朝向。
   - `can_add_face` 拒绝退化三角形与非流形边（每条无向边最多 2 个面）。
   - 通过量化顶点键（`quant=1e10`）去重，减少浮点误差导致的裂缝。

## 关键代码展示

### 边界体元判定与八叉树插入

```cpp
unsigned char value = 0;
for (int pi = 0; pi < 8; ++pi)
{
  int gx = x + kPointDeltaCS[pi][0];
  int gy = y + kPointDeltaCS[pi][1];
  int gz = z + kPointDeltaCS[pi][2];
  if (!pointInsideCached(gx, gy, gz))
    value |= kPointFlagCS[pi];
}
if (value != 0 && value != 255)
{
  OctreeNode *leaf = create_to_leaf(x, y, z);
  leaf->parms.valid = true;
  leaf->parms.config = value;
  leaf->parms.d = calculate_d(x, y, z, value);
}
```

### Shrink 合并判据

```cpp
bool can_merge_node(OctreeNode *node, int &D) const
{
  unsigned char normalType = kNormalNotSimple;
  bool found = false;
  for (int i = 0; i < 8; ++i)
  {
    OctreeNode *c = node->children[i];
    if (c != NULL)
    {
      if (!c->parms.valid) return false;
      unsigned char nt = kConfigToNormalTypeId[c->parms.config];
      if (nt == kNormalNotSimple) return false;
      if (!found) { found = true; normalType = nt; D = c->parms.d; }
    }
  }
  if (!found) return false;
  for (int i = 0; i < 8; ++i)
  {
    OctreeNode *c = node->children[i];
    if (c != NULL)
    {
      unsigned char nt = kConfigToNormalTypeId[c->parms.config];
      if (nt != normalType || c->parms.d != D) return false;
    }
  }
  return true;
}
```

### 角点顺序重映射（实现顺序 -> 标准 MC 顺序）

```cpp
static inline unsigned char remap_cfg_to_mc(unsigned char cfg)
{
  unsigned char out = 0;
  for (int i = 0; i < 8; ++i)
  {
    if (cfg & (1 << i))
      out |= static_cast<unsigned char>(1 << kCsToMcCorner[i]);
  }
  return out;
}
```

### 边交点线性插值

```cpp
CPoint intersect_edge(const CPoint &p0, const CPoint &p1) const
{
  double f0 = m_implicitFunc(p0) - m_isovalue;
  double f1 = m_implicitFunc(p1) - m_isovalue;
  if (fabs(f1 - f0) < 1e-12 || f0 * f1 > 0)
    return (p0 + p1) * 0.5;
  double t = -f0 / (f1 - f0);
  t = std::max(0.0, std::min(1.0, t));
  return p0 + (p1 - p0) * t;
}
```

### 面片法向一致性与非流形约束

```cpp
CPoint n = (p1 - p0) ^ (p2 - p0);
if (n.norm() <= 1e-10) continue;
CPoint triCenter = (p0 + p1 + p2) / 3.0;
CPoint grad = gradient(triCenter);
if ((n * grad) < 0) std::swap(p1, p2);

if (can_add_face(tri, edgeUse, dirEdgeUse))
  out->createFace(tri, fid++);
```

## 可视化结果展示

![](image.jpg)

## 总结分析

1. 本实现属于“八叉树组织下的稳健 MC 提取”：通过边界体元筛选 + shrink 压缩树结构，减少无效计算。
2. `NormalTypeId + D` 是 shrink 判据核心，可保证合并过程的平面一致性。
3. 三角片生成阶段采用逐体元 MC，并通过角点重映射与梯度校正保证面方向一致。
4. 线性插值交点比简单边中点更准确，能提升几何质量。
5. `can_add_face` 中的退化检测与边使用次数约束可抑制非流形和重复面。

## 参考文献

1. Lorensen, W. E., Cline, H. E. Marching Cubes: A High Resolution 3D Surface Construction Algorithm. SIGGRAPH 1987.
2. Montani, C., Scateni, R., Scopigno, R. Discretized Marching Cubes. Graphics Interface 1994.
3. Chernyaev, E. V. Marching Cubes 33: Construction of Topologically Correct Isosurfaces. 1995.
4. chnhideyoshi. 基于八叉树的网格生成算法剖析. https://www.cnblogs.com/chnhideyoshi/p/OctreeBasedSimplifiedMarchingCubes.html
