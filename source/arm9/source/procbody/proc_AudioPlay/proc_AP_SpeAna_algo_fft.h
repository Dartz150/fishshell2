
#include <math.h>

#define FFT_Samples (1024/2)

/***********************************************************
    fft.c -- FFT (����Fourier��Q)
***********************************************************/
#define PI (3.14159265358979323846)

DATA_IN_IWRAM_AudioPlay static float HummingWindow[FFT_Samples];

DATA_IN_IWRAM_AudioPlay static float *psintbl=NULL; /* �ӥåȷ�ܞ�� */
DATA_IN_IWRAM_AudioPlay static s32 *pbitrev=NULL; /* �����v���� */

/*
  �v��{\tt fft()}����Ո���Ȥ��������v���������.
*/
static void FFT_Init_ins_make_sintbl(void)
{
    s32 i, n2, n4, n8;
    double c, s, dc, ds, t;

    n2 = FFT_Samples / 2;  n4 = FFT_Samples / 4;  n8 = FFT_Samples / 8;
    t = sin(PI / FFT_Samples);
    dc = 2 * t * t;  ds = sqrt(dc * (2 - dc));
    t = 2 * dc;  c = psintbl[n4] = 1;  s = psintbl[0] = 0;
    
    for (i = 1; i < n8; i++){
        c -= dc;  dc += t * c;
        s += ds;  ds -= t * s;
        psintbl[i] = s;  psintbl[n4 - i] = c;
    }
    
    if (n8 != 0) psintbl[n8] = sqrt(0.5);
    
    for (i = 0; i < n4; i++){
        psintbl[n2 - i] = psintbl[i];
    }
    
    for (i = 0; i < n2 + n4; i++){
        psintbl[i + n2] = - psintbl[i];
    }
}

/*
  �v��{\tt fft()}����Ո���Ȥ��ƥӥåȷ�ܞ�������.
*/
static void FFT_Init_ins_make_bitrev(void)
{
    const s32 n2 = FFT_Samples / 2;
    
    s32 i=0,j=0;
    
    for ( ; ; ) {
        pbitrev[i] = j;
        if (++i >= FFT_Samples) break;
        s32 k = n2;
        while (k <= j) {
          j -= k;  k /= 2;
        }
        j += k;
    }
}

static void FFT_Init_ins_CreateHummingWindow(void)
{
  for(u32 idx=0;idx<FFT_Samples;idx++){
    float hw=0.54 - 0.46 * cos(2*PI*idx/(FFT_Samples-1));
    HummingWindow[idx]=hw;
  }
}

static void FFT_Init(void)
{
    s32 n4=FFT_Samples / 4;
    
    psintbl = (float*)safemalloc(&MM_Process,(FFT_Samples + n4) * sizeof(float));
    if(psintbl==NULL) StopFatalError(0,"Fatal error: FFT: psintbl memory overflow.\n");
    FFT_Init_ins_make_sintbl();
    
    pbitrev = (s32*)safemalloc(&MM_Process,FFT_Samples * sizeof(s32));
    if(pbitrev==NULL) StopFatalError(0,"Fatal error: FFT: pbitrev memory overflow.\n");
    FFT_Init_ins_make_bitrev();
    
    FFT_Init_ins_CreateHummingWindow();
}

static void FFT_Free(void)
{
  if(psintbl!=NULL){
    safefree(&MM_Process,psintbl); psintbl=NULL;
  }
  if(pbitrev!=NULL){
    safefree(&MM_Process,pbitrev); pbitrev=NULL;
  }
}

/*
  ����Fourier��Q (Cooley--Tukey�Υ��르�ꥺ��).
  �˱������ {\tt n} ��2�������\���ޤ�.
  {\tt x[$k$]} ���g��, {\tt y[$k$]} ���鲿 ($k = 0$, $1$, $2$,
  \ldots, $|{\tt n}| - 1$).
  �Y���� {\tt x[]}, {\tt y[]} ���ϕ��������.
  ${\tt n} = 0$ �ʤ��Υ�����Ť���.
  ${\tt n} < 0$ �ʤ����Q���Ф�.
  ǰ�ؤȮ��ʤ� $|{\tt n}|$ �΂��Ǻ��ӳ�����,
  �����v���ȥӥåȷ�ܞ�α�����뤿��˶�����֤˕r�g��������.
  ���α�Τ����ӛ���I��@�ä�ʧ�������1�򷵤� (�����K�˕r
  �Α��ꂎ��0).
  �����α��ӛ���I����Ť���ˤ� ${\tt n} = 0$ �Ȥ���
  ���ӳ��� (���ΤȤ��� {\tt x[]}, {\tt y[]} �΂��ω���ʤ�).
*/

static void FFT_Exec(float *px, float *py)
{
    /* �ʂ� */
    const s32 n4 = FFT_Samples / 4;
    
    /* �ӥåȷ�ܞ */
    for (u32 i = 0; i < FFT_Samples; i++) {
        s32 j = pbitrev[i];
        if (i < j) {
            float t;
            t = px[i];  px[i] = px[j];  px[j] = t;
            t = py[i];  py[i] = py[j];  py[j] = t;
        }
    }
    
    /* ��Q */
    for (s32 k = 1; k < FFT_Samples;) {
        s32 h = 0;
        s32 k2=k+k;
        s32 d = FFT_Samples / k2;
        for (s32 j = 0; j < k; j++) {
            const float c = psintbl[h + n4];
            const float s = psintbl[h];
            for (s32 i = j; i < FFT_Samples; i += k2) {
                const s32 ik = i + k;
                const float dx = s * py[ik] + c * px[ik];
                const float dy = c * py[ik] - s * px[ik];
                px[ik] = px[i] - dx;  px[i] += dx;
                py[ik] = py[i] - dy;  py[i] += dy;
            }
            h += d;
        }
        k=k2;
    }
    
    /* ���Q�Ǥʤ��ʤ�n�Ǹ�� */
    for (u32 i = 0; i < FFT_Samples; i++) {
      px[i] /= FFT_Samples;  py[i] /= FFT_Samples;
    }
}

