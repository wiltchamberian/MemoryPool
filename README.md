# MemoryPool
an implement of a memorypool

一个简单而又强大的内存池实现
1个头文件+1个cpp实现

语法采用c++

#QuickMemoryPool
* @brief : QuickMemoryPool
* 一种简单高效的模板定长内存池实现
* 采用预分配大块内存机制，减少malloc,free等系统call开销
* 时间复杂度:分配内存O(1),释放内存O(1)
* 该内存池实现参考Box2D的内存池实现，用模板参数取代了固定长度,并做了一些细节方面的优化
* 支持任意类型对象(POD,含虚函数的等)，支持构造函数，含参构造函数， 自动触发析构函数
* 为了保持实现的简洁和高效性，该内存池仅仅提供两个接口，申请到的指针需要用户自己做管理
* 简单测试下，其速度相比不使用内存池的申请释放，速度达到近3倍
* 
*  内存池结构图
内存池有多个Chunk组成，
Chunk0 ,Chunk1, Chunk2,....每个Chunk结构相同

单个Chunk结构由多个block组成,Block里存放着用户数据
Chunk:[Block0,Block1,....          


