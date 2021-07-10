#include<stdio.h>
#include<Windows.h>
#include<math.h>
#include<tchar.h>
#include<time.h>

//����BPMҪ�õ���ֵ
#define BPM 60000
//���������ܴﵽ��λ�ã���ʼ���ã�
#define INF -100

typedef struct NOTE {
    int num;    //����������λ��
    int t;        //������ʱ��
    NOTE *next;    //��һ�����
} Note;

//��λ����
char f[] = {'Z', '1', 'X', '2', 'C', 'V', '3', 'B', '4', 'N', '5', 'M', 'A', '6', 'S', '7', 'D', 'F', '8', 'G', '9',
            'H', '0', 'J', 'Q', 'K', 'W', 'L', 'E', 'R', 'O', 'T', 'P', 'Y', static_cast<char>(188), 'U', 'I'};
int bpm = 0;    //����
int noteSet = 0;//��ȡ�׺ź�������λ��
Note *tail;        //β��㣨β�巨��������
int out = 0;    //����ff14�������
int count = 0;

//�ж���ǰ�Ĵ����Ƿ�Ϊff14��Ϸ����
int isFF14Window() {
    HWND hwnd = GetForegroundWindow();
    char gameWindowText[] = "���ջ���XIV";
    wchar_t windowText[256];
    GetWindowText(hwnd, reinterpret_cast<LPSTR>(windowText), 256);
    if (!_tcscmp(reinterpret_cast<const char *>(windowText), reinterpret_cast<const char *>(gameWindowText)))
        return 1;
    return 0;
}

//�޸��׺�
void SetClef(FILE *fp) {
    char b, c[3] = {0};
    int i;
    for (i = 0, b = fgetc(fp); b != '\n'; i++, b = fgetc(fp)) {
        c[i] = b;
    }
    noteSet = 12;    //C��
    switch (c[0]) {
        case 'C':
            break;
        case 'D':
            noteSet += 2;
            break;
        case 'E':
            noteSet += 4;
            break;
        case 'F':
            noteSet += 5;
            break;
        case 'G':
            noteSet += 7;
            break;
        case 'A':
            noteSet += 9;
            break;
        case 'B':
            noteSet += 11;
            break;
    }
    switch (c[1]) {
        case '#':
            noteSet += 1;
            break;
        case 'b':
            noteSet -= 1;
            break;
    }
}

//�޸�BPM
int SetBPM(FILE *fp) {
    int i, j;
    char b;
    char fbpm[3];
    for (i = 0, b = fgetc(fp); b != '\n'; i++, b = fgetc(fp))    //���ļ������ȡfbpm
        fbpm[i] = b - 48;
    int bpm = 0;                    //��ʼ��bpm
    for (j = 0; j < i; j++)
        bpm += pow(10, i - j - 1) * fbpm[j];        //��fbpm��ȡbpm
    bpm = BPM / bpm;                    //����ÿһ�ĵ�ʱ�䣬��λms
    return bpm;
}

//��ʼ��ͷ���
Note *InitNote(Note *&p) {
    p = (Note *) malloc(sizeof(Note));
    p->next = NULL;
    return p;
}

//β�巨������
void InsertNote(int t, int num) {
    Note *p;
    InitNote(p);
    p->num = num;
    int temp = num - 36;
    if (temp > out) {
        printf("%d �������� %d\n", count, temp);
        out = temp;
    }
    count += 1;
    p->t = t;
    tail->next = p;
    tail = tail->next;
}

//��������
void SetPitch(char b, int *pitch) {
    *pitch = 0;
    switch (b) {
        case '0':
            *pitch = INF;
        case '1':
            break;
        case '2':
            *pitch += 2;
            break;
        case '3':
            *pitch += 4;
            break;
        case '4':
            *pitch += 5;
            break;
        case '5':
            *pitch += 7;
            break;
        case '6':
            *pitch += 9;
            break;
        case '7':
            *pitch += 11;
            break;    //��������
    }
}

//��������
void CreateList(FILE *fp, Note *&list) {
    out = 0;
    count = 0;
    InitNote(list);
    tail = list;
    printf("����������...\n");
    SetClef(fp);
    bpm = SetBPM(fp);
    char b, a;
    for (b = fgetc(fp); b != EOF; b = fgetc(fp))    //��ȡ����
    {
        if (b == '\n') {    //�ı��׺ź�BPM
            SetClef(fp);
            bpm = SetBPM(fp);
            continue;
        }
        int t = bpm, delay = 1, pitch = 0, multiNotesCount = 0;
        SetPitch(b, &pitch);
        for (a = fgetc(fp); a != ' ' && a != EOF; a = fgetc(fp)) {
            switch (a) {
                case '`':
                    pitch += 12;
                    break;        //����1���˶�
                case '.':
                    pitch -= 12;
                    break;        //�½�һ���˶�
                case '_':
                    t /= 2;
                    delay += 1;
                    break;    //ʱֵ���루��ʼΪ�ķ�������ʱֵ��
                case '-':
                    t += bpm;
                    break;        //ʱֵ��һ���ķ�������ʱֵ
                case '#':
                    pitch += 1;
                    break;        //����
                case 'b':
                    pitch -= 1;
                    break;        //����
                case ':':
                    t += bpm / (2 * delay);
                    break;    //��������
                default:
                    InsertNote(20, pitch + noteSet);
                    SetPitch(a, &pitch);
                    multiNotesCount += 1;
            }
        }
        t = t - multiNotesCount * 20 > 20 ? t - multiNotesCount : t;
        InsertNote(t, pitch + noteSet);
    }
    printf("����������ɣ�\n");
    fclose(fp);
}

// "����˯��"
void wakeSleep(long milliSec) {
    struct timeb start;
    ftime(&start);
    struct timeb end;
    while (1) {
        ftime(&end);
        if (end.millitm + end.time * 1000 - start.time * 1000 - start.millitm > milliSec) {
            return;
        }
    }
}

//������ת���ɰ����ź�
void ClickList(Note *list) {
    Sleep(100);
    printf("��ʼ����\n");
    Note *p = list->next;
    while (p != NULL) {
        if (!isFF14Window()) {
            printf("����ֹͣ\n");
            return;
        }
        if (p->num < 0) {
            wakeSleep(p->t);
            p = p->next;
            continue;
        }
        keybd_event(f[p->num], 0, 0, 0);
        // û�з�װ�ɺ���������Ƶ��������ջ֡������������ʧ����Ҫ�����������װ���� (ʵ���װ�ɺ���Ҫ��1ms)
        struct timeb start;
        struct timeb end;
        ftime(&start);
        while (1) {
            ftime(&end);
            if (end.millitm + end.time * 1000 - start.time * 1000 - start.millitm > p->t - (p->t) / 4) {
                break;
            }
        }
//        Sleep(p->t - (p->t) / 4);
        keybd_event(f[p->num], 0, 2, 0);
        ftime(&start);
        while (1) {
            ftime(&end);
            if (end.millitm + end.time * 1000 - start.time * 1000 - start.millitm > (p->t) / 4) {
                break;
            }
        }
//        Sleep((p->t) / 4);
        p = p->next;
    }
    printf("�������\n");
}

//ͨ�������������ӵĵ�ǿ�ưѳ�������������ff14������
void LimitNote(Note *&list) {
    printf("������ %d\n", out);
    Note *p = list;
    while (p != NULL) {
        p->num -= out;
        if (p->num < 0) {
            p->num = INF;
        }
        p = p->next;
    }
}

//��������
void DestroyList(Note *&list) {
    Note *p = list->next;
    while (p != NULL) {
        free(list);
        list = p;
        p = p->next;
    }
    free(list);
    list = NULL;
}

int main() {
    system("title ff14����ver1.5");
    system("mode con cols=70 lines=40");
    char a[100];
    FILE *fp;
    while (true) {
        system("cls");
        for (int i = 0; i < 100; i++)
            a[i] = '\0';
        printf("\n play text file��");
        gets(a);
        fp = fopen(a, "r");    //���ı��ļ�
        if (fp == NULL)
            printf("\n cannot find the file: %s\n", a);
        else {
            printf("----�л������ջ���14��Ϸ�п�ʼ����----");
            while (!isFF14Window())
                Sleep(100);    //ÿ0.1����ff14���ڣ���⵽�˾��˳�ѭ��

            Note *list;
            CreateList(fp, list);
            LimitNote(list);
            ClickList(list);
            DestroyList(list);
        }
        system("pause");
    }
}