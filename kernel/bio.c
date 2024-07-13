// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"



#define HASH_SIZE 13

//定义散列桶
struct hash_table{
  struct spinlock lock;
  struct buf head;
};


struct hash_table hash_table[HASH_SIZE];


struct {
 //DEBUG
  //struct spinlock lock;

  struct buf buf[NBUF];
 
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
 
 

//  struct buf head;
} bcache;







void
binit(void)
{
  struct buf *b;

  ;

  
  char lockname[20];
  for(int i =0; i < HASH_SIZE; i++)
    {
      snprintf(lockname, sizeof(lockname), "bcache_%d", i);
      initlock(&hash_table[i].lock, lockname);
      hash_table[i].head.prev = &hash_table[i].head;
      hash_table[i].head.next = &hash_table[i].head;
    }

  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){    //头插法
    b->next = hash_table[0].head.next;
    b->prev = &hash_table[0].head;
    initsleeplock(&b->lock, "buffer");
    hash_table[0].head.next->prev = b;
    hash_table[0].head.next = b;
  }


  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   initsleeplock(&b->lock, "buffer");
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;
  // }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  //acquire(&bcache.lock);
  int hash_blockno = blockno % HASH_SIZE;
  acquire(&hash_table[hash_blockno].lock);
  for(b = hash_table[hash_blockno].head.next; b != &hash_table[hash_blockno].head; b = b->next)
  {
      if(b->dev == dev && b->blockno == blockno){
        b->refcnt++;

         acquire(&tickslock);
        b->ticks = ticks;
        release(&tickslock);

        release(&hash_table[hash_blockno].lock);
        acquiresleep(&b->lock);
        return b;
      }
  }




  //原子的


  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.



    b = 0;
    struct buf* tmp;
    for(int i = hash_blockno, cycle = 0; cycle != HASH_SIZE; i = (i + 1) % HASH_SIZE)
    {
      ++cycle;
      if(i != hash_blockno){
        if(!holding(&hash_table[i].lock))
          acquire(&hash_table[i].lock);
      }
      for(tmp = hash_table[i].head.next; tmp != &hash_table[i].head; tmp = tmp->next)
      {
          if(tmp->refcnt == 0 && (b == 0 || tmp->ticks < b->ticks))
            b = tmp;
      }
    
    if(b){
      if(i != hash_blockno){
           b->next->prev = b->prev;
           b->prev->next = b->next;
           release(&hash_table[i].lock);

           b->next = hash_table[hash_blockno].head.next;
           b->prev = &hash_table[hash_blockno].head;
           hash_table[hash_blockno].head.next->prev = b;
           hash_table[hash_blockno].head.next = b;
      }
    
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;

      acquire(&tickslock);
      b->ticks = ticks;
      release(&tickslock);

      release(&hash_table[hash_blockno].lock);
      acquiresleep(&b->lock);
      return b;
     
    }else {
        if(i != hash_blockno)
        {
            release(&hash_table[i].lock);
        }
    }


    }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");


  
  int hash_blockno = b->blockno % HASH_SIZE;
  releasesleep(&b->lock);
  acquire(&hash_table[hash_blockno].lock);
    b->refcnt--;
    acquire(&tickslock);
    b->ticks = ticks;
    release(&tickslock);
  release(&hash_table[hash_blockno].lock);
  

}

void
bpin(struct buf *b) {
  int hash_blockno = b->blockno % HASH_SIZE;
  acquire(&hash_table[hash_blockno].lock);
    b->refcnt++;
  release(&hash_table[hash_blockno].lock);
}

void
bunpin(struct buf *b) {
  int hash_blockno = b->blockno % HASH_SIZE;
  acquire(&hash_table[hash_blockno].lock);
    b->refcnt--;
  release(&hash_table[hash_blockno].lock);

}


