# MemoryPool
an implement of a memorypool

一个简单而又强大的内存池实现
1个头文件+1个cpp即可直接实现

语法采用c++
demo如下

ace::MemoryPool pool;

int *a = pool.malloc<int>();
  
//MemoryPool析构时将真正释放所有内存

