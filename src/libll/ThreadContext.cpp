
#if !(defined MIPS_EMUL)

#include <string.h>

#include "icode.h"
#endif
#include "ThreadContext.h"
#include "globals.h"
#if !(defined MIPS_EMUL)
#include "opcodes.h"
#endif

#ifdef TASKSCALAR
#include "MemBuffer.h"
#include "TaskContext.h"
#endif

#include "Events.h"

#if (defined TLS)
#include "Epoch.h"
#endif

extern icode_ptr Idone1;         /* calls terminator1() */
// void malloc_share(thread_ptr child, thread_ptr parent);

Pid_t ThreadContext::baseActualPid;
Pid_t ThreadContext::nextActualPid;
ThreadContext::ContextVector ThreadContext::actualPool;

Pid_t ThreadContext::baseClonedPid;
Pid_t ThreadContext::nextClonedPid;
ThreadContext::ContextVector ThreadContext::clonedPool;

size_t ThreadContext::nThreads;

ThreadContext::ContextVector ThreadContext::pid2context;

ThreadContext *ThreadContext::mainThreadContext;

void ThreadContext::staticConstructor(void)
{
  nextActualPid=baseActualPid=0;
  nextClonedPid=baseClonedPid=1024;

  mainThreadContext=newActual();
  nThreads = 1;
}

void ThreadContext::copy(const ThreadContext *src)
{
  parent=src->parent;
  youngest=src->youngest;
  sibling=src->sibling;
#if (defined MIPS_EMUL)
  addressSpace=src->addressSpace;
#else // For (defined MIPS_EMUL)
  for(size_t i=0;i<32;i++) {
    reg[i]=src->reg[i];
    fp[i]=src->fp[i];
  }

  lo=src->lo;
  hi=src->hi;

  fcr31=src->fcr31;
  picode=src->picode;
  target=src->target;

  // Copy address mapping info
  virtToRealOffset=src->virtToRealOffset;

  // Copy virtual address ranges
  dataVAddrLb=src->dataVAddrLb;
  dataVAddrUb=src->dataVAddrUb;
  allStacksAddrLb=src->allStacksAddrLb;
  allStacksAddrUb=src->allStacksAddrUb;
#endif // For else of (defined MIPS_EMUL)
  myStackAddrLb=src->myStackAddrLb;
  myStackAddrUb=src->myStackAddrUb;
}

ThreadContext *ThreadContext::getContext(Pid_t pid)
{
  I(pid>=0);
  I((size_t)pid<pid2context.size());
  return pid2context[pid];
}

size_t ThreadContext::size() 
{
  return nThreads;
}

ThreadContext *ThreadContext::getMainThreadContext(void)
{
  I(mainThreadContext);
  I(mainThreadContext->pid==0);
  I(pid2context.size()>0);
  I(pid2context[0]==mainThreadContext);
  return mainThreadContext;
}

ThreadContext *ThreadContext::newActual(void)
{
  ThreadContext *context;
  if(actualPool.empty()) {
#if !(defined MIPS_EMUL)
    I(nextActualPid<Max_nprocs);
#endif
    context=static_cast<ThreadContext *>(calloc(1,sizeof(ThreadContext)));
    // Initialize the actual context for the first time
    context->pid=nextActualPid++;
    //    context->pll_lock=&LL_lock[context->pid];
    context->fd=(char *)malloc(MAX_FDNUM*sizeof(char));
    I(context->fd);

    if(pid2context.size()<(size_t)nextActualPid)
      pid2context.resize(nextActualPid,0);
  }else{
    context=actualPool.back();
    actualPool.pop_back();
  }
  pid2context[context->pid]=context;
#if (defined TLS)
  context->myEpoch=0;
#endif
  nThreads++;
  return context;
}

ThreadContext *ThreadContext::newActual(Pid_t pid)
{
  I(pid2context[pid]==0);
  ContextVector::iterator poolIt=actualPool.begin();
  while((*poolIt)->pid!=pid) {
    I(poolIt!=actualPool.end());
    poolIt++;
  }
  ThreadContext *context=*poolIt;
  *poolIt=actualPool.back();
  actualPool.pop_back();
  I(context->pid==pid);
  pid2context[pid]=context;
#if (defined TLS)
  context->myEpoch=0;
#endif
  nThreads++;
  return context;
}

ThreadContext *ThreadContext::newCloned(void)
{
  ThreadContext *context;
  if(clonedPool.empty()) {
    context=static_cast<ThreadContext *>(calloc(1,sizeof(ThreadContext)));
    context->pid=nextClonedPid++;
    pid2context.resize(nextClonedPid,0);
  }else{
    context=clonedPool.back();
    clonedPool.pop_back();
  }
  pid2context[context->pid]=context;
#if (defined TLS)
  context->myEpoch=0;
#endif
  nThreads++;
  return context;
}  

void ThreadContext::free(void)
{
  // The main context has PID zero and should not be freed until the end
  // If it is freed it can be reused, and no child process should have
  // pid zero because it can not be distinguished from the parent in
  // fork-like function calls (which return child's pid in the parent and
  // zero in the child process).
  I((this!=mainThreadContext)||(nThreads==1));
  I(pid>=0);
  I((size_t)pid<pid2context.size());
  I(pid2context[pid]==this);
  // Stupid hack needed to prevent the main thread's address space from
  // ever being freed. When the last thread is freed, some memory requests are
  // still pending and check for validity of their addresses using the
  // main thread context. This needs to be fixed: 1) the thread should die
  // only when its last instruction retires, and 2) a dinst should know
  // its thread, becasue with multiple address spaces it is not enough to
  // know the vaddr and use the main thread to check vaddr validity
  if(this==mainThreadContext)
    return;
  pid2context[pid]=0;
#if (defined MIPS_EMUL)
  if(!isCloned()){
    // Delete stack memory from address space
    addressSpace->deleteSegment(myStackAddrLb,myStackAddrUb-myStackAddrLb);
  }
  // Detach from address space
  addressSpace->delReference();
  addressSpace=0;
#else
  heapManager->delReference();
  heapManager=0;
#endif // (defined MIPS_EMUL)
  if(isCloned()) {
    clonedPool.push_back(this);
  }else{
    actualPool.push_back(this);
  }
  nThreads--;
}

#if !(defined MIPS_EMUL)
void ThreadContext::initAddressing(VAddr dataVAddrLb2, VAddr dataVAddrUb2,
				   MINTAddrType rMap, MINTAddrType mMap, MINTAddrType sTop)
{
  dataVAddrLb=dataVAddrLb2;
  dataVAddrUb=dataVAddrUb2;
  allStacksAddrLb=Stack_start;
  allStacksAddrUb=Stack_end;
  myStackAddrLb = sTop;
  myStackAddrUb = myStackAddrLb + Stack_size;
  // Segments order: (Rdata|Data)...Bss...Heap...Stacks
  if(Data_start <= Rdata_start) {
    virtToRealOffset = mMap;
  }else{
    virtToRealOffset = rMap;
  }
    
  // Rdata should be in the static data area
  I(Rdata_start >= dataVAddrLb &&
    Rdata_end <= heapManager->getHeapAddrLb());
  // Data should be in the static data area
  I(Data_start >=  dataVAddrLb &&
    Data_end <= heapManager->getHeapAddrLb());
  // BSS should be in the static data area
  I(Bss_start >= dataVAddrLb &&
    (Bss_start + Bss_size) <= heapManager->getHeapAddrLb());
  // Heap should be between static and stack areas
  I(allStacksAddrLb>=heapManager->getHeapAddrUb());
  // Stack should be in the data area, too
  I(allStacksAddrUb<=dataVAddrUb);
  
  MSG("static[0x%x-0x%x] heap[0x%x-0x%x] stack[0x%x-0x%x] -> [%p-%p]"
      ,(VAddr)dataVAddrLb,(VAddr)(Bss_start+Bss_size)
      ,(VAddr)(heapManager->getHeapAddrLb())
      ,(VAddr)(heapManager->getHeapAddrUb())
      ,(VAddr)allStacksAddrLb
      ,(VAddr)allStacksAddrUb
      ,(void*)(dataVAddrLb+virtToRealOffset)
      ,(void*)(allStacksAddrUb+virtToRealOffset)
      );
}
#endif // !(defined MIPS_EMUL)

#if !(defined MIPS_EMUL)
void ThreadContext::dump()
{
  int i, j;

  printf("thread 0x%p:\n", this);
  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i++)
      printf("  r%d: %s 0x%08x", i, i < 10 ? " " : "", reg[i]);
    printf("\n");
  }
  printf("  lo:   0x%08x  hi:   0x%08x\n", lo, hi);

  /* print out floats and doubles */
  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i++)
      printf("  $f%d:%s %10.6f", i, i < 10 ? " " : "", getFPNUM(i));
    printf("\n");
  }

  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i += 2)
      printf("  $d%d:%s %10.6f", i, i < 10 ? " " : "", getDPNUM(i));
    printf("\n");
  }

  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i++)
      printf("  $w%d:%s 0x%08x", i, i < 10 ? " " : "", getWFPNUM(i));
    printf("\n");
  }

  printf("  fcr0 = 0x%x, fcr31 = 0x%x\n", fcr0, fcr31);

  printf("  target = 0x%p, pid = %d\n", target, pid);

  printf("  parent = 0x%p, youngest = 0x%p, sibling = 0x%p\n",
         parent, youngest,  sibling);
}

/* dump the stack for stksize number of words */
void ThreadContext::dumpStack()
{
  int stksize = Stack_size;
  VAddr sp;
  VAddr i;
  int j;
  unsigned char c;

  sp = getREGNUM(29);
  printf("sp = 0x%08x\n", (unsigned) sp);
  for (i = sp + (stksize - 1) * 4; i >= sp; i -= 4) {
    printf("0x%08x (+%d): 0x%x (%f)  ",
           i,  (i - sp), *(int *) this->virt2real(i), *(float *) this->virt2real(i));
    for (j = 0; j < 4; j++) {
      c = *((char *) this->virt2real(i) + j);
      if (c < ' ')
        c += 'A' - 1;
      else if (c >= 0x7f)
        c = '?';
      printf("^%c", c);
    }
    printf("\n");
  }
}
#endif // !(defined MIPS_EMUL)

void ThreadContext::useSameAddrSpace(thread_ptr pthread)
{
#if (defined MIPS_EMUL)
  setAddressSpace(pthread->getAddressSpace());
#else // For (defined MIPS_EMUL)
  setHeapManager(pthread->getHeapManager());
  
  lo = pthread->lo;
  hi = pthread->hi;
  for (int i = 0; i < 32; i++)
    reg[i] = pthread->reg[i];


  fcr0 = pthread->fcr0;
  fcr31 = pthread->fcr31;
  for (int i = 0; i < 32; i++)
    fp[i] = pthread->fp[i];

  dataVAddrLb=pthread->dataVAddrLb;
  dataVAddrUb=pthread->dataVAddrUb;
  allStacksAddrLb=pthread->allStacksAddrLb;
  allStacksAddrUb=pthread->allStacksAddrUb;

  myStackAddrLb=pthread->myStackAddrLb;
  myStackAddrUb=pthread->myStackAddrUb;

  virtToRealOffset=pthread->virtToRealOffset;

#if (defined TLS) || (defined TASKSCALAR)
  fd = pthread->fd;
#endif
#endif // For else of (defined MIPS_EMUL)
}

void ThreadContext::shareAddrSpace(thread_ptr pthread, int share_all, int copy_stack)
{
  // The address space has already been allocated

  I(share_all); // share_all is the only supported

#if (defined MIPS_EMUL)
  setAddressSpace(pthread->getAddressSpace());
//   // Create my stack space and map it into my address space
//   size_t stackSize=Stack_size;    
//   VAddr  stackStart=addressSpace->findVMemHigh(stackSize);
//   addressSpace->newRMem(stackStart,stackStart+stackSize);
//   setStack(stackStart,stackStart+stackSize);
#else // For (defined MIPS_EMUL)
  setHeapManager(pthread->getHeapManager());

  /* copy all the registers */
  for (int i = 0; i < 32; i++)
    reg[i] = pthread->reg[i];
  lo = pthread->lo;
  hi = pthread->hi;
  for (int i = 0; i < 32; i++)
    fp[i] = pthread->fp[i];
  fcr0 = pthread->fcr0;
  fcr31 = pthread->fcr31;

  dataVAddrLb=pthread->dataVAddrLb;
  dataVAddrUb=pthread->dataVAddrUb;
  allStacksAddrLb=pthread->allStacksAddrLb;
  allStacksAddrUb=pthread->allStacksAddrUb;

  myStackAddrLb=allStacksAddrLb+pid*Stack_size;
  myStackAddrUb=myStackAddrLb+Stack_size;
  I(myStackAddrUb<=allStacksAddrUb);

  virtToRealOffset=pthread->virtToRealOffset;

  if (copy_stack) {
#if (defined TLS)
    // Need to use TLS read/write methods to properly copy the stack
    I(!copy_stack);
#endif
    // Compute the child's stack pointer and copy the used portion of the stack
    VAddr srcStkPtr=pthread->getStkPtr();
    I(pthread->isLocalStackData(srcStkPtr));
    VAddr dstStkPtr=myStackAddrLb+(srcStkPtr-pthread->myStackAddrLb);
    setStkPtr(dstStkPtr);
    I(isLocalStackData(dstStkPtr));
    memcpy((void *)(virt2real(dstStkPtr)),
	   (void *)(pthread->virt2real(srcStkPtr)),
	   myStackAddrUb-dstStkPtr);
  } else {
    // Point to the top of the stack (but leave some empty space there)
    setStkPtr(myStackAddrUb-FRAME_SIZE);
    I(isLocalStackData(getStkPtr()));
  }
#endif // For else of (defined MIPS_EMUL)
}

#ifdef TASKSCALAR
void ThreadContext::badSpecThread(VAddr addr, short opflags) const
{
  I(pid != -1);

  GLOG(pid == -1, "failed Addressing Invalid thread");

  if(!rsesc_is_safe(pid) && rsesc_is_versioned(pid)) {
    LOG("speculative thread using bad pointer. stopping thread. pid = %d. Iaddr=0x%08x"
        , pid, osSim->eventGetInstructionPointer(pid)->addr);
    rsesc_exception(pid);
    return;
  }

  I(0); // warning, bad code?

  PAddr spawnAddr = 0xdeadbeef; // not valid address
  spawnAddr = TaskContext::getTaskContext(pid)->getSpawnAddr();

  bool badAddr  = !isValidDataVAddr(addr);
  bool badAlign = (
                   MemBufferEntry::calcAccessMask(opflags
                                                  ,MemBufferEntry::calcChunkOffset(addr))
                   ) == 0;

  GMSG(badAlign,"(failed) bad aligment 0x%3x [flags=0x%x] in safe thread. pid=%d. pc=0x%08x  (R31=0x%08x) (SA=0x%08x)"
       , (int)addr, (int)opflags, pid, osSim->eventGetInstructionPointer(pid)->addr, osSim->getContextRegister(pid, 31),
       spawnAddr);


  GMSG(badAddr, "(failed) bad pointer 0x%3x [flags=0x%x] in safe thread. pid=%d. pc=0x%08x  (R31=0x%08x) (SA=0x%08x)" 
       , (int)addr, (int)opflags, pid, osSim->eventGetInstructionPointer(pid)->addr, osSim->getContextRegister(pid, 31),
       spawnAddr);

  if(!badAddr)  // temporary ISCA03 hack
    return;

  mint_termination(pid);
}
#endif

void ThreadContext::init()
{
  youngest  = NULL;
#if !(defined MIPS_EMUL)
  int i;

  reg[0]    = 0;
  rerrno    = 0;
  youngest  = NULL;

  /* map in the global errno */
  if (Errno_addr)
    perrno = (int *) virt2real(Errno_addr);
  else
    perrno = &rerrno;

  /* init all file descriptors to "closed" */
  for (i = 0; i < MAX_FDNUM; i++)
    fd[i] = 0;
#endif // For !(defined MIPS_EMUL)
}

void ThreadContext::initMainThread()
{
  int i, entry_index;
  thread_ptr pthread=getMainThreadContext();

  pthread->parent = NULL;
  pthread->sibling = NULL;

#if !(defined MIPS_EMUL)
  Maxpid = 0;

  pthread->init();

#ifdef MIPS2_FNATIVE
  pthread->fcr31 = s_get_fcr31();
#else
  pthread->fcr31 = 0;
#endif

  /* Set the picode to the first executable instruction. */
  if (Text_entry == 0)
    Text_entry = Text_start;
  entry_index = (Text_entry - Text_start) / sizeof(int);
  pthread->picode = Itext[entry_index];

  /* initialize the icodes for the terminator functions */
  /* set up this picode so that terminator1() gets called */
  Idone1 = Itext[Text_size + DONE_ICODE];
  Idone1->func = terminator1;
  Idone1->opnum = terminate_opn;

  /* Set up the return address so that terminator1() gets called.
   * This probably isn't necessary since exit() gets called instead */
  pthread->reg[31]=(int)icode2addr(Idone1);
#endif // For !(defined MIPS_EMUL)
}

void ThreadContext::newChild(ThreadContext *child)
{
  child->parent = this;
 
  child->sibling = youngest;
  youngest = child;
}

Pid_t ThreadContext::getThreadPid(void) const{
#if (defined TLS)
  I(getEpoch());
  return getEpoch()->getTid();
#else
  return getPid();
#endif
}

#if !(defined MIPS_EMUL)
unsigned long long ThreadContext::getMemValue(RAddr p, unsigned dsize) {
  unsigned long long value = 0;
  switch(dsize) {
  case 1:
    value = *((unsigned char *) p);
    break;
  case 2:
    value = *((unsigned short *) p);
#ifdef LENDIAN
    value = SWAP_SHORT(value);
#endif      
    break;
  case 4:
    value = *((unsigned *) p);
#ifdef LENDIAN
    value = SWAP_WORD(value);
#endif      
    break;
  case 8:
    value = *((unsigned long long*) p);
#ifdef LENDIAN
    value = SWAP_LONG(value);
#endif      
    break;
  default:
    MSG("ThreadContext:warning, getMemValue with bad (%d) data size.", dsize);
    value = 0;
    break;
  }
  
  return value;
}

int MintFuncArgs::getInt32(void) {
  int retVal; 
  I(sizeof(retVal)==4);
  I((curPos % 4)==0);
  if(curPos<16){
    I(curPos % 4==0);
    retVal=myContext->getIntReg((IntRegName)(4+curPos/4));
  }else{
    RAddr addr=myContext->virt2real(myContext->getStkPtr())+curPos;
#if (defined TASKSCALAR) || (defined TLS)
    int *ptr =(int *)(rsesc_OS_read(myContext->getPid(),myIcode->addr,addr,E_WORD));
#else
    int *ptr=(int *)addr;
#endif
    retVal=SWAP_WORD(*ptr);
  }                                     
  curPos+=4;
  return retVal;
}           

long long int MintFuncArgs::getInt64(void){
  long long int retVal;
  I(sizeof(retVal)==8);
  I((curPos%4)==0);
  // Align current position                  
  if(curPos%8!=0)
    curPos+=4;               
  I(curPos%8==0);
  if(curPos<16){
    retVal=myContext->getIntReg((IntRegName)(4+curPos/4));
    retVal=(retVal<<32)&0xFFFFFFFF00000000llu;
    retVal|=myContext->getIntReg((IntRegName)(4+curPos/4+1))&0xFFFFFFFFllu;
  }else{
    RAddr addr=myContext->virt2real(myContext->getStkPtr())+curPos;
#if (defined TASKSCALAR) || (defined TLS)
    long long int *ptr =
      (long long int *)(rsesc_OS_read(myContext->getPid(),myIcode->addr,addr,E_DWORD));
#else
    long long int *ptr=(long long int*)addr;
#endif
    retVal=SWAP_LONG(*ptr);
  }
  curPos+=8;
  return retVal;
}
#endif // !(defined MIPS_EMUL)

#if (defined MIPS_EMUL)
void ThreadContext::writeMemFromBuf(VAddr addr, size_t len, void *buf){
  I(canWrite(addr,len));
  uint8_t *byteBuf=(uint8_t *)buf;
  while(len){
    if((addr&sizeof(uint8_t))||(len<sizeof(uint16_t))){
      writeMemRaw(addr,*((uint8_t *)byteBuf));
      addr+=sizeof(uint8_t);
      byteBuf+=sizeof(uint8_t);
      len-=sizeof(uint8_t);
    }else if((addr&sizeof(uint16_t))||(len<sizeof(uint32_t))){
      writeMemRaw(addr,*((uint16_t *)byteBuf));
      addr+=sizeof(uint16_t);
      byteBuf+=sizeof(uint16_t);
      len-=sizeof(uint16_t);
    }else if((addr&sizeof(uint32_t))||(len<sizeof(uint64_t))){
      writeMemRaw(addr,*((uint32_t *)byteBuf));
      addr+=sizeof(uint32_t);
      byteBuf+=sizeof(uint32_t);
      len-=sizeof(uint32_t);
    }else{
      I(!(addr%sizeof(uint64_t)));
      I(len>=sizeof(uint64_t));
      writeMemRaw(addr,*((uint64_t *)byteBuf));
      addr+=sizeof(uint64_t);
      byteBuf+=sizeof(uint64_t);
      len-=sizeof(uint64_t);
    }
  }
}
ssize_t ThreadContext::writeMemFromFile(VAddr addr, size_t len, int fd){
  I(canWrite(addr,len));
  ssize_t retVal=0;
  uint8_t buf[AddressSpace::getPageSize()];
  while(len){
    size_t ioSiz=AddressSpace::getPageSize()-(addr&(AddressSpace::getPageSize()-1));
    if(ioSiz>len) ioSiz=len;
    ssize_t nowRet=read(fd,buf,ioSiz);
    if(nowRet==-1)
      return nowRet;
    retVal+=nowRet;
    writeMemFromBuf(addr,nowRet,buf);
    addr+=nowRet;
    len-=nowRet;
    if(nowRet<(ssize_t)ioSiz)
      break;
  }
  return retVal;
}
void ThreadContext::writeMemWithByte(VAddr addr, size_t len, uint8_t c){
  I(canWrite(addr,len));
  uint8_t buf[AddressSpace::getPageSize()];
  memset(buf,c,AddressSpace::getPageSize());
  while(len){
    size_t wrSiz=AddressSpace::getPageSize()-(addr&(AddressSpace::getPageSize()-1));
    if(wrSiz>len) wrSiz=len;
    writeMemFromBuf(addr,wrSiz,buf);
    addr+=wrSiz;
    len-=wrSiz;
  }
}
void ThreadContext::readMemToBuf(VAddr addr, size_t len, void *buf){
  I(canRead(addr,len));
  uint8_t *byteBuf=(uint8_t *)buf;
  while(len){
    if((addr&sizeof(uint8_t))||(len<sizeof(uint16_t))){
      readMemRaw(addr,*((uint8_t *)byteBuf));
      addr+=sizeof(uint8_t);
      byteBuf+=sizeof(uint8_t);
      len-=sizeof(uint8_t);
    }else if((addr&sizeof(uint16_t))||(len<sizeof(uint32_t))){
      readMemRaw(addr,*((uint16_t *)byteBuf));
      addr+=sizeof(uint16_t);
      byteBuf+=sizeof(uint16_t);
      len-=sizeof(uint16_t);
    }else if((addr&sizeof(uint32_t))||(len<sizeof(uint64_t))){
      readMemRaw(addr,*((uint32_t *)byteBuf));
      addr+=sizeof(uint32_t);
      byteBuf+=sizeof(uint32_t);
      len-=sizeof(uint32_t);
    }else{
      I(!(addr%sizeof(uint64_t)));
      I(len>=sizeof(uint64_t));
      readMemRaw(addr,*((uint64_t *)byteBuf));
      addr+=sizeof(uint64_t);
      byteBuf+=sizeof(uint64_t);
      len-=sizeof(uint64_t);
    }
  }
}
ssize_t ThreadContext::readMemToFile(VAddr addr, size_t len, int fd){
  I(canRead(addr,len));
  ssize_t retVal=0;
  uint8_t buf[AddressSpace::getPageSize()];
  while(len){
    size_t ioSiz=AddressSpace::getPageSize()-(addr&(AddressSpace::getPageSize()-1));
    if(ioSiz>len) ioSiz=len;
    readMemToBuf(addr,ioSiz,buf);
    ssize_t nowRet=write(fd,buf,ioSiz);
    if(nowRet==-1)
      return nowRet;
    retVal+=nowRet;
    addr+=nowRet;
    len-=nowRet;
    if(nowRet<(ssize_t)ioSiz)
      break;
  }
  return retVal;
}
ssize_t ThreadContext::readMemString(VAddr stringVAddr, size_t maxSize, char *dstStr){
  size_t i=0;
  while(true){
    char c;
    if(!readMem(stringVAddr+i,c))
      return -1;
    if(i<maxSize)
      dstStr[i]=c;
    i++;
    if(c==(char)0)
      break;
  }
  return i;
}
#endif // (define MIPS_EMUL)
