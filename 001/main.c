//
//  main.c
//  des
//
//  Created by 白家栋 on 2020/9/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define MAX_GROUP 10000
// ----------------------- 全局变量区 ---------------------------------------------
int group_num;

char BASE64_ENCODING_TABLE [64] =
{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

int Sbox[8][4][16] =
{
    // S1-box
    {
        {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
        {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
        {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
        {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}
    },
    
    // S2-box
    {
        {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
        {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
        {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
        {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}
    },
    
    // S3-box
    {
        {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
        {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
        {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
        {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}
    },
    
    // S4-box
    {
        {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
        {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
        {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
        {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}
    },
    
    // S5-box
    {
        {2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
        {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
        {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
        {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}
    },
    
    // S6-box
    {
        {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
        {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
        {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
        {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0 ,8, 13}
    },
    
    // S7-box
    {
        {4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
        {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
        {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
        {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}
    },
    
    // S8-box
    {
        {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
        {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
        {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
        {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
    }
};

int IP[8][8] =
{
    {58, 50, 42, 34, 26, 18, 10, 2},
    {60, 52, 44, 36, 28, 20, 12, 4},
    {62, 54, 46, 38, 30, 22, 14, 6},
    {64, 56, 48, 40, 32, 24, 16, 8},
    {57, 49, 41, 33, 25, 17, 9, 1},
    {59, 51, 43, 35, 27, 19, 11, 3},
    {61, 53, 45, 37, 29, 21, 13, 5},
    {63, 55, 47, 39, 31, 23, 15, 7}
};

int PC_1[8][7] =
{
    {57, 49, 41, 33, 25, 17, 9},
    {1, 58, 50, 42, 34, 26, 18},
    {10, 2, 59, 51, 43, 35, 27},
    {19, 11, 3, 60, 52, 44, 36},
    {63, 55, 47, 39, 31, 23, 15},
    {7, 62, 54, 46, 38, 30, 22},
    {14, 6, 61, 53, 45, 37, 29},
    {21, 13, 5, 28, 20, 12, 4}
};

int PC_2[8][6] =
{
    {14, 17, 11, 24, 1, 5},
    {3, 28, 15, 6, 21, 10},
    {23, 19, 12, 4, 26, 8},
    {16, 7, 27, 20, 13, 2},
    {41, 52, 31, 37, 47, 55},
    {30, 40, 51, 45, 33, 48},
    {44, 49, 39, 56, 34, 53},
    {46, 42, 50, 36, 29, 32}
};

int LEFT_MOV_2 [4][7] =
{
    {3, 4, 5, 6, 7, 8, 9},
    {10, 11, 12, 13, 14, 15, 16},
    {17, 18, 19, 20, 21, 22, 23},
    {24, 25, 26, 27, 28, 1, 2}
};

int LEFT_MOV_1[4][7] =
{
    {2, 3, 4, 5, 6, 7, 8},
    {9, 10, 11, 12, 13, 14, 15},
    {16, 17, 18, 19, 20, 21, 22},
    {23, 24, 25, 26, 27, 28, 1}
};

int E_EXTENSION[8][6] =
    {
        {32, 1, 2, 3, 4, 5},
        {4, 5, 6, 7, 8, 9},
        {8, 9, 10, 11, 12, 13},
        {12, 13,14,15,16,17},
        {16, 17,18, 19, 20, 21},
        {20, 21, 22, 23, 24, 25},
        {24, 25, 26, 27, 28, 29},
        {28, 29, 30, 31, 32, 1}
    };

int P_TRANSFER[4][8] =
{
    {16, 7, 20, 21, 29, 12, 28, 17},
    {1, 15, 23, 26, 5, 18, 31, 10},
    {2, 8, 24, 14, 32, 27, 3, 9},
    {19, 13, 30, 6, 22, 11, 4, 25},
};

int IP_INVERSE[8][8] =
{
    {40, 8, 48, 16, 56, 24, 64, 32},
    {39, 7, 47, 15, 55, 23, 63, 31},
    {38, 6, 46, 14, 54, 22, 62, 30},
    {37, 5, 45, 13, 53, 21, 61, 29},
    {36, 4, 44, 12, 52, 20, 60, 28},
    {35, 3, 43, 11, 51, 19, 59, 27},
    {34, 2, 42, 10, 50, 18, 58, 26},
    {33, 1, 41, 9, 49, 17, 57, 25}
};


// ---------------------------------------------------------------------------------





//  -------------- tool, 用于辅助des的实现的函数, 在算法中没有实际的效力 -------------------
#define min(X, Y) ((X) < (Y) ? (X) : (Y))
#define LEN(arr) ((int) (sizeof (arr) / sizeof(arr[0])))



int CEIL(int length, int div) {                 // 取上界
    return ((length % div == 0 ? length / div : length / div + 1));
}


void print_byte_as_bits(char val)
{
  for (int i = 7; i >=0; i--)
  {
    printf("%c ", (val & (1 << i)) ? '1' : '0');
  }
}

int get_bit_by_position(int index, char * input) {
    index--;
    int i = index/8;
    int j = 7 - index%8;
    return (input[i] & (1 << j) ? 1 : 0);
}

// 7个bit一组的数组中根据序号获取某一位
int get_bit_by_position_7(int index, char * input) {
    index--;
    int i = index/7;
    int j = 6 - index%7;
    return (input[i] & (1 << j) ? 1 : 0);
}

// 通过行号列号获取某一位
int get_bit_by_row_col(int i, int j, char * input) {
    j = 7 - j;
    return (input[i] & (1 << j) ? 1 : 0);
}

// 通过行号列号设置某一位的值
void set_bit_by_row_col(int i, int j, char * input, int bit) {
    j = 7 - j;
    input[i] = input[i] | (bit << j);
}


// 打印一组值(由28位，48位，56位，64位组成)，用于调试
void printM(char input[], int length) {
    for (int i = 0; i < length; i++)
    {
      print_byte_as_bits(input[i]);
      printf("\n");
    }
}

// 从一个M中得到L0
void Li(char * input, char * l0) {
    for(int i = 0;i<4;i++) {
        l0[i] = '\0';
        l0[i] = input[i];
    }
}

// 从一个M中得到R0
void Ri(char * input, char * r0) {
    for(int i = 0;i<4;i++) {
        r0[i] = '\0';
        r0[i] = input[i+4];
    }
}

void xor_by_bit_32(char * l, char * f) {
    for(int i = 0;i<4;i++) {
        l[i] ^= f[i];
    }
}

void xor_by_bit(char * l, char * f, int len) {
    for(int i = 0;i<len;i++) {
        l[i] ^= f[i];
    }
}

// 将输入的6个bit的值根据6转4的规则来转换成4位
char convert_6bit_2_4bit_by_sbox(char bit6, int box_index){
    char bit4 = 0;
    
    int n = (bit6 & 1 ? 1 : 0)  + (bit6 & (1 << 5) ? 1 : 0) * 2;
    
    int m = (bit6 & (1 << 1) ? 1 : 0) + (bit6 & (1 << 2) ? 1 : 0) * 2 + (bit6 & (1 << 3) ? 1 : 0) * 4 + (bit6 & (1 << 4) ? 1 : 0) * 8;
    
    bit4 = Sbox[box_index][n][m];
    return bit4;
}

// 左移两位
void left_mov_2_bit(char * input, char * output) {
    for(int i = 0;i<4;i++) {
        output[i] = '\0';
    }
    
    for(int i = 0;i<4;i++) {
        
        for(int j = 0;j<7;j++) {
            int index = LEFT_MOV_2[i][j];
            int bit = get_bit_by_position_7(index, input);
            set_bit_by_row_col(i, j+1, output, bit);
        }
    }
}

// 左移1位
void left_mov_1_bit(char * input, char * output) {
    for(int i = 0;i<4;i++) {
        output[i] = '\0';
    }
    
    for(int i = 0;i<4;i++) {
        for(int j = 0;j<7;j++) {
            int index = LEFT_MOV_1[i][j];
            int bit = get_bit_by_position_7(index, input);
            set_bit_by_row_col(i, j+1, output, bit);
        }
    }
}

void convert_3_to_4(char * input, char * output) {
    
    for(int i = 0;i<4;i++) {
        output[i] = '\0';
    }
    int k = 1;
    for(int i = 0;i<4;i++) {
        for(int j = 0;j<8;j++) {
            if(j == 0 || j == 1) {
                set_bit_by_row_col(i, j, output, 0);
            } else {
                int bit = get_bit_by_position(k, input);
                k++;
                set_bit_by_row_col(i, j, output, bit);
            }
        }
    }
}

int convert_char_to_base64(char * input, int len, char *output) {
    
    int i = 0;
    int num_of_equal = (len % 3 == 0) ? 0 : 3 - (len % 3);
    int index = 0;
    while(i < len) {
        char t[3];
        if(len-i >= 3) {
            t[0] = input[i];
            t[1] = input[i+1];
            t[2] = input[i+2];
           
        } else {
            int fill = 3 - (len-i);
            int j = 0;
            for(;j<len - i;j++) {
                t[j] = input[j+i];
            }
            
            for(int k = 0;k < fill;k++) {
                t[j+k] = 0;
            }
        }
        char temp[4];
        convert_3_to_4(t, temp);
        for(int j = 0;j<4;j++) {
            output[index] = BASE64_ENCODING_TABLE[temp[j]];
            index++;
        }
        i += 3;
    }
    
    for(int i = 0;i<num_of_equal;i++) {
        output[index - i -1] = '=';
    }
    return index;
}
// ----------------------------------------------------------------------------------



// ------------用于初始化明文，将明文划分为若干组，每组有64位(也就是8个字节或者8个char)----------
void initialize(char* input, int length, char minput[MAX_GROUP][8]) {   // 初始化DES分组
    
      // 计算组数
    length--;
    group_num = CEIL(length, 8);

      // 如果不需要填充就能成功分组，则需要在最后一组添加值为8的8个char
    if(length%8==0) {
        for(int i = 0;i<8;i++) {
            minput[group_num][i] = 8;
        }
        group_num++;
    }
  
      // 每8个char分一组，不足8个则通过要添加的个数作为值来填充
    int groupIndex = 0;
    for(int i = 0;i<length;) {
        int times = min(8, length - i);
        int j = 0;
        while(j < times) {
            minput[groupIndex][j++] = input[i++];
        }
        short fill = 8-j;
        while(j < 8) {
            minput[groupIndex][j++] = fill;
        }
        groupIndex++;
    }
}

void test_initialization()    // 用于测试initialize函数
{
    char testString[] = "hello, world";
    int len = LEN(testString);
    char minput[20][8];
    initialize(testString, len, minput);
  for (int i = 0; i < group_num; i++)
  {
      printf("M: %d\n", i);

    for (int j = 0; j < 8; j++)
    {
      print_byte_as_bits(minput[i][j]);
      printf("\n");
    }
    printf("-------\n\n");
  }
}
// ----------------------------------------------------------------------------------



// ------------------------  初始置换IP以及测试 ----------------------------------------
void initialIPTransfer(char* m, char* m0) {
    
    for(int i = 0;i<8;i++) {
        m0[i] = '\0';
    }
    for(int i = 0;i < 8;i++) {        // L0
        for(int j = 0;j < 8;j++) {
            int index = IP[i][j];
            int bit = get_bit_by_position(index, m);
            set_bit_by_row_col(i, j, m0, bit);
        }
    }
}

void test_initialIPTransfer() {
    char test[] = "abcdefgh";
    char input[20][8];
    initialize(test, LEN(test), input);
    printf("before the IP transfer:\n");
    
    printM(input[0], 8);
    
    printf("\n\nafter the IP transfer:\n");
    char m0[8];
    initialIPTransfer(input[0], m0);
    printM(m0, 8);
}
// ----------------------------------------------------------------------------------



// ------------------------  生成密钥算法 ----------------------------------------

// input为64位的K，outcome为8 * 7
void rowTransmision(char * input, char * outcome) {
    for(int i = 0;i<8;i++) {
        outcome[i] = '\0';
    }
    for(int i = 0;i<8;i++) {
        
        for(int j = 0;j<7;j++) {
            int index = PC_1[i][j];
            int bit = get_bit_by_position(index, input);
            
            set_bit_by_row_col(i, j+1, outcome, bit);
        }
    }
}

//K为给定的64位密钥, ki为16个要生成的子密钥, ki的大小为48位
void produceK16(char * K, char ki[17][8]) {
    // 进行行置换
    char k_pc[8];   // 8 * 7
    rowTransmision(K, k_pc);

    char C0[4], D0[4];              // 得到c0与d0,都为4 * 7

    for(int i = 0;i<4;i++) {
        C0[i] = k_pc[i];
    }
    for(int i = 4;i<8;i++) {
        D0[i-4] = k_pc[i];
    }
    
    char prev_c[4], prev_d[4];      // 记录Ci-1与Di-1，作为下一次的输入
    for(int i = 0;i<4;i++) {
        prev_c[i] = C0[i];
        prev_d[i] = D0[i];
    }
    
    for(int k = 1;k<=16;k++) {      // 生成16个子密钥
        char Ci[4], Di[4];
        
        if(k == 1 || k == 2 || k == 9 || k == 16) {     // 如果为1, 2, 9, 16则mov1位
            left_mov_1_bit(prev_c, Ci);
            left_mov_1_bit(prev_d, Di);
        } else {
            left_mov_2_bit(prev_c, Ci);
            left_mov_2_bit(prev_d, Di);
        }
        
        
        for(int i = 0;i<4;i++) {                // 更新prev_c, prev_d
            prev_c[i] = Ci[i];
            prev_d[i] = Di[i];
        }
        
        char CDi[8];                            // 拼接Ci, Di为CDi，为8 * 7
        for(int i = 0;i<4;i++) {
            CDi[i] = Ci[i];
        }
        for(int i = 4;i<8;i++) {
            CDi[i] = Di[i-4];
        }
        
        
        char t[8];                                  // 进行PC_2的置换, 作为密钥，为8 * 6
        for(int i = 0;i<8;i++) {
            t[i] = '\0';
        }
        for(int i = 0;i<8;i++) {
            
            for(int j = 0;j<6;j++) {
                int index = PC_2[i][j];
                int bit = get_bit_by_position_7(index, CDi);
                set_bit_by_row_col(i, j+2, t, bit);
            }

        }
        for(int i = 0;i<8;i++) {                // 赋值给密钥
            ki[k][i] = t[i];
        }

    }
}

void test_produceK16() {
    
    char key[] = "abcdefgh";
    char input[20][8];
    initialize(key, LEN(key), input);
    printf("\n\nfinal test: 测试子密钥\n");
    
    char k[16][8];

    produceK16(input[0], k);
    
    for(int i = 0;i<16;i++) {
        printf("\n密钥K%d\n", i);
        printM(k[i], 8);
        printf("\n");
    }
}
// -----------------------------------------------------------------------------






// ------------------------ Feistel 轮函数 f(Ri-1, Ki)，E-扩展 -------------------------------

// e扩展，r_i为32位，e_r_i为扩展后的48位串（实际上是一个 8*6 的bit矩阵, 从右往左数两列都没有意义）
void e_extension(char * r_i, char * e_r_i) {
    for(int i = 0;i<8;i++) {
        e_r_i[i] = '\0';
    }
    
    for(int i = 0;i<8;i++) {
        for(int j = 0;j<6;j++) {
            int bit = get_bit_by_position(E_EXTENSION[i][j], r_i);
            set_bit_by_row_col(i, j+2, e_r_i, bit);
        }
    }
}

// input是64位(其中仅8*6位有效), outcome为32位, 4个char
void sboxTransfer(char * input, char * outcome) {
    for(int i = 0;i<4;i++) {
        int boxIndex = i * 2;
        outcome[i] = '\0';
        outcome[i] = convert_6bit_2_4bit_by_sbox(input[boxIndex], boxIndex);
        boxIndex++;
        outcome[i] = convert_6bit_2_4bit_by_sbox(input[boxIndex], boxIndex) | outcome[i] << 4;
    }
}

// input为4 * 8，outcome为4 * 8
void P_Transfer(char* input, char* outcome) {
    for(int i = 0;i<4;i++) {
        outcome[i] = '\0';
    }
    
    for(int i = 0;i<4;i++) {
        for(int j = 0;j<8;j++) {
            int index = P_TRANSFER[i][j];
            int bit = get_bit_by_position(index, input);
            set_bit_by_row_col(i, j, outcome, bit);
        }
    }
}

// R_i_1为32位(4个字节/4个char)，K_i为48位子密钥(8个字节/8个char)，outcome为32位(4个字节/4个char)
void feistel(char* R_i_1, char* K_i, char* outcome) {

    char e_ext[8];
    e_extension(R_i_1, e_ext);
    
 
    // 与密钥进行异或, e_ext为48位, k_i为48位
    xor_by_bit(e_ext, K_i, 8);
    
    char sbox_outcome[4];
    // sbox转换, e_ext为
    sboxTransfer(e_ext, sbox_outcome);
    
    P_Transfer(sbox_outcome, outcome);
}

void test_feistel() {
    char testString[] = "abcdefghi";
    char t[20][8];
    initialize(testString, LEN(testString), t);
    char * m = t[0];
    char r0[4];
    Ri(m, r0);
    
    char k[16][8];

    produceK16(m, k);
    
    char testOutcome[4];
    feistel(r0, k[1], testOutcome);
}

void test_e_extension() {
    char testString[] = "abcdefghi";
    char t[20][8];
    initialize(testString, LEN(testString), t);
    
    printf("original M: \n");
    char * m = t[0];
    printM(m, 8);
    
    printf("\nR0 of M: \n");
    char r0[4];
    Ri(m, r0);

    printM(r0, 4);
    
    printf("\nafter E extension: \n");
    char e_ext[8];
    
    e_extension(r0, e_ext);
    printM(e_ext, 8);
}

void test_sboxTransfer() {
    char testString[] = "abcdefghi";
    char t[20][8];
    initialize(testString, LEN(testString), t);
    printf("original M: \n");
    char * m = t[0];
    printM(m, 8);
    
    printf("\nR0 of M: \n");
    char r0[4];
    Ri(m, r0);
    printM(r0, 4);
    
    printf("\nafter E extension: \n");
    char e_ext[8];
    
    e_extension(r0, e_ext);
    printM(e_ext, 8);
    
    
    char outcome[4];
    
    sboxTransfer(e_ext, outcome);
    
    printf("\nafter the sbox extension: \n");
    printM(outcome, 4);
    
    char p_t[4];
    printf("\nafter p transfer: \n");
    P_Transfer(outcome, p_t);
    printM(p_t, 4);
    
}
// ----------------------------------------------------------------------------------



// -------------------- iterationT 迭代序列T，和迭代16次的函数以及测试---------------------
// L_i_1为32位(4个字节/4个char), R_i_1为32位(4个字节/4个char)，K_i为48位子密钥(6个字节/6个char)，mi为64位(8个字节/8个char)


// m0为经过IP置换的块，64位；K为生成好的16个子密钥，每个子密钥有48位；m16为最终的结果，应当为64位
void iteration16T(char * m0, char K[16][8], char *m16) {
    char L0[4], R0[4];
    char prev_L[4], prev_R[4];
    
    // 初始化L0, R0
    Li(m0, L0);
    Ri(m0, R0);

    // 用prev_L, prev_R来保存上一个L0和R0, 作为计算Li, Ri的输入
    for(int i = 0;i<4;i++) {
        prev_L[i] = L0[i];
        prev_R[i] = R0[i];
    }
    
    
    for(int k = 1;k<=16;k++) {
        // Li, Ri为要生成的Li与Ri
        
        char Li[4], Ri[4];
        for(int i = 0;i<4;i++) {
            Li[i] = prev_R[i];
        }
        
        
        char f[4];
        // feistel轮函数，输入为R_{i-1}, 密钥K
        feistel(prev_R, K[k], f);
        
        
        for(int i = 0;i<4;i++) {
            Ri[i] = prev_L[i];
        }
        
        // 轮函数的结果与Ri异或
        xor_by_bit_32(Ri, f);
        
        // 记录Li与Ri
        for(int i = 0;i<4;i++) {
            prev_L[i] = Li[i];
            prev_R[i] = Ri[i];
        }
    }
    
    // 将Li与Ri拼接，得到 mi
    for(int i = 0;i<8;i++) {
        m16[i] = '\0';
        if(i < 4) {
            m16[i] = prev_R[i];
        } else {
            m16[i] = prev_L[i-4];
        }
    }
    
    
}
// ----------------------------------------------------------------------------------

// ------------------------ ip 逆置换 ------------------------------------------------
// input为64位，outcome为64位
void ipInverseTransfer(char * input, char * outcome) {
    for(int i = 0;i<8;i++) {
        outcome[i] = '\0';
    }
    
    for(int i = 0;i<8;i++) {
        for(int j = 0;j<8;j++) {
            int index = IP_INVERSE[i][j];
            int bit = get_bit_by_position(index, input);
            set_bit_by_row_col(i, j, outcome, bit);
        }
    }
}

void test_ipInverseTransfer() {
    char testString[] = "abcdefghi";
    char t[20][8];
    initialize(testString, LEN(testString), t);
    
    printf("original M: \n");
    char * m = t[0];
    printM(m, 8);
    char inv_m[8];
    
    ipInverseTransfer(m, inv_m);
    printf("\nafter inverse ip:\n");
    printM(inv_m, 8);
}
// ----------------------------------------------------------------------------------



// -------------------------- operate 单个 M -------------------------------------------
void operateOneM(char * M, char * outcome, char K[16][8]) {
    
    
    char m0[8];
    initialIPTransfer(M, m0);        // 初始置换IP
    
    char m16[8];
    iteration16T(m0, K, m16);        // 16轮迭代与交换置换
    
    ipInverseTransfer(m16, outcome); // 逆置换
}

void test_operateOneM() {
    char testString[] = "abcdefghi";
    char t[20][8];
    initialize(testString, LEN(testString), t);
    
    char * m = t[0];
    char k[16][8];

    produceK16(m, k);
    
    char outcome[8];
    operateOneM(m, outcome, k);
    
}
// ------------------------------------------------------------------------------------


// -------------------------- DES 完整操作汇总 ------------------------------------------
void DES(char * input, int length, char * secret, char outcome[MAX_GROUP][8]) {
    
    // 密钥
    char k[17][8];
    for(int i = 0;i<17;i++) {
        for(int j = 0;j<8;j++) {
            k[i][j] = '\0';
        }
    }
    // 生成子密钥
    produceK16(secret, k);

    printf("\n");
    // 存放分组的数据结构
    char groups[MAX_GROUP][8];
    
    // 初始化分组
    initialize(input, length, groups);
    
    // 对每一个分组执行操作
    for(int i = 0;i<group_num; i++) {
        char t[8];
        operateOneM(groups[i], t, k);
        
        // 对分组结束操作后，将密文写入结果
        for(int j = 0;j<8;j++) {
            outcome[i][j] = t[j];
        }
    }
}
// ------------------------------------------------------------------------------------
int main()
{
    // 明文长度最大限制为 80000, 可以修改MAX_GROUP变量的值来修改这个限制
    char input[] =
    "*Astronomy* in Elizabethan times was much closer to what we would nowadays term astrology. It was not yet weighted down with knowledge of what the planets and stars actually are, as modern day astronomy is. There was a widespread belief that the stars, in their various conjunctions, had an important and direct influence on the life of humans, both on individuals, and on social institutions. See the sonnet by Sidney, given at the bottom of the page. He calls those who consider the stars to shine merely to spangle the night 'dusty wits', for to him their importance was much greater. They were an importance influence in human lives. Although his sonnet, like this one, by its conclusion is somewhat tongue in cheek. (Note that Sidney uses the term astrology. He also reads Stellas's eyes as if they were stars). The poet here claims to 'have Astronomy', i.e he understands it as a science, and then he proceeds to tell us how his knowledge differs from that of the traditional astrologer (lines 3-8).We tend to think of ourselves as a more rational age, but a recent president of the United States, Ronald Reagan, relied on his wife's astrologer to forecast for him propitious days for work and policy decisions.";
    int len = LEN(input);
    // 密钥长度为8个字符，超出部分无效
    char secret[] = "abcdefgh";

    char outcome[MAX_GROUP][8];
    for(int i = 0;i<MAX_GROUP;i++) {
        for(int j = 0;j<8;j++) {
            outcome[i][j] = '\0';
        }
    }
    

    DES(input, len, secret, outcome);

    char secret_input[MAX_GROUP * 8];
    int length = 0;
    for(int i = 0;i<group_num;i++) {
        for(int j = 0;j<8;j++) {
            secret_input[length++] = outcome[i][j];
        }
    }
    printf("明文: %s\n", input);
    printf("64位密钥: %s\n", secret);
    printf("密文的8位有符号数形式: ");
    for(int i = 0;i<length;i++) {
        printf("%d ", secret_input[i]);
    }
    printf("\n密文的base64编码后的形式: ");
    char base64_output[MAX_GROUP];
    int len_of_base64 = convert_char_to_base64(secret_input, length, base64_output);
    for(int i = 0;i<len_of_base64;i++) {
        printf("%c", base64_output[i]);
    }
    printf("\n");
}
