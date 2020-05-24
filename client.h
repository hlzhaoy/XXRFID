#ifdef __cplusplus
extern "C" {
#endif

int StartClient(XXRFIDCLient *s);
int ReadClient(XXRFIDCLient *s, unsigned char* rbuf, int len);
int WriteClient(XXRFIDCLient *s, unsigned char* rbuf, int len);
int ClearClient(XXRFIDCLient *s);
#ifdef __cplusplus
}
#endif