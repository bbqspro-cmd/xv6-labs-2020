// 物理内存分配器
// 用于给：
// 1. 用户进程分配内存
// 2. 内核栈分配空间
// 3. 页表页分配空间
// 4. pipe缓冲区分配空间
//
// xv6采用页(Page)作为最小分配单位
// 每页大小为4096字节(PGSIZE)
#include "types.h"     // xv6自定义数据类型，如uint64
#include "param.h"     // 系统参数，如PHYSTOP
#include "memlayout.h" // 内存布局定义
#include "spinlock.h"  // 自旋锁
#include "riscv.h"     // RISC-V相关寄存器操作
#include "defs.h"      // 内核函数声明

// 释放一段物理地址范围内的所有页
void freerange(void *pa_start, void *pa_end);

// kernel.ld链接脚本定义
// end表示内核代码+数据结束后的第一个地址
//
// end之后的物理内存区域属于空闲内存
extern char end[];

// 空闲物理页链表节点
//
// xv6没有额外创建链表节点结构
// 而是直接把空闲页的第一页空间当作链表节点
//
// 当页空闲时：
//
// 物理页:
// +----------------+
// | struct run     |
// | next指针       |
// |                |
// | 空闲空间       |
// +----------------+
//
// 分配出去以后，这块空间直接给用户使用
struct run
{
  struct run *next; // 指向下一个空闲物理页
};

// 内核物理内存管理结构
struct
{
  struct spinlock lock; // 自旋锁，保护freelist并发访问

  struct run *freelist; // 空闲物理页链表头指针

} kmem;

// 初始化物理内存分配器
void kinit()
{
  // 初始化自旋锁
  // 防止多个CPU同时操作空闲页链表
  initlock(&kmem.lock, "kmem");

  // 将kernel结束地址到PHYSTOP之间的内存
  // 加入空闲链表
  //
  // end:
  // +----------------+
  // | kernel代码     |
  // | kernel数据     |
  // +----------------+
  //
  // end以后：
  // +----------------+
  // | 空闲物理内存   |
  // +----------------+
  freerange(end, (void *)PHYSTOP);
}

// 将一段连续物理地址范围加入空闲链表
void freerange(void *pa_start, void *pa_end)
{
  char *p;

  // 将起始地址向上对齐到页边界
  //
  // 例如：
  //
  // pa_start = 0x80001001
  //
  // PGROUNDUP后:
  //
  // p = 0x80002000
  //
  // 因为物理页必须从页边界开始
  p = (char *)PGROUNDUP((uint64)pa_start);

  // 每次移动一个页大小
  // 将每一页释放
  //
  // p + PGSIZE <= pa_end
  // 保证释放的是完整页面
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// 释放一个物理页
//
// 参数:
// pa: 要释放的物理地址
//
// 释放后:
// 该页加入kmem.freelist链表
void kfree(void *pa)
{
  struct run *r;

  // 检查地址是否合法
  //
  // 条件1:
  // 必须页对齐
  //
  // 条件2:
  // 不能释放kernel之前的区域
  //
  // 条件3:
  // 不能超过物理内存最大地址
  if (((uint64)pa % PGSIZE) != 0 ||
      (char *)pa < end ||
      (uint64)pa >= PHYSTOP)

    panic("kfree");

  // 将释放页填充垃圾数据
  //
  // 目的:
  // 防止访问已经释放的内存
  //
  // 如果程序错误使用释放后的指针
  // 更容易发现bug
  memset(pa, 1, PGSIZE);

  // 把当前页转换成链表节点
  //
  // 注意：
  // 没有额外分配struct run
  // 直接利用该物理页存next指针
  r = (struct run *)pa;

  // 加锁
  acquire(&kmem.lock);
  // 头插法加入空闲链表
  //
  // 原:
  //
  // freelist
  //    |
  //    v
  //    A -> B -> C
  // 插入:
  //
  // freelist
  //    |
  //    v
  //    R -> A -> B -> C
  r->next = kmem.freelist;
  kmem.freelist = r;
  // 解锁
  release(&kmem.lock);
}
// 分配一个4096字节物理页
//
// 返回:
// 成功:
//     返回物理地址
//
// 失败:
//     返回0
void *
kalloc(void)
{
  struct run *r;

  // 获取链表锁
  acquire(&kmem.lock);

  // 取空闲链表第一个页
  r = kmem.freelist;

  // 删除链表头节点
  if (r)
    kmem.freelist = r->next;
  // 释放锁
  release(&kmem.lock);

  // 如果分配成功
  if (r)
  {
    // 填充垃圾数据
    // 作用:
    // 避免程序依赖初始化后的0值
    memset((char *)r, 5, PGSIZE);
  }
  // 返回物理页地址
  return (void *)r;
}
// 统计当前剩余空闲物理内存大小
//
// 返回:
// 空闲页数量 * 4096
//
// 供sysinfo系统调用使用
uint64
freemem_bytes(void)
{
  uint64 bytes = 0;

  struct run *r;

  // 遍历空闲页链表
  for (r = kmem.freelist; r; r = r->next)
  {
    // 每个节点代表一个4096字节物理页
    bytes += PGSIZE;
  }

  return bytes;
}