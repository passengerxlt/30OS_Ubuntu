
void io_hlt(void);

void HariMain(void)
{
fin:
  io_hlt(); /*执行asmfunc.asm里的_io_hlt*/
  goto fin;
}
