#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_LENGTH 1024
typedef struct student
{
    int xuehao;
    char name[30];
    int Class;
    char sex[6];
    float ZGPA;
    float KGPA;
}Student;
typedef struct course
{
    int daihao;
    char name[30];
    float GPA;
    float xuefen;
    float chengji;
}Course;
typedef struct Lcourse
{
    struct course data;
    struct Lcourse *next;
}CourseListNode;
struct Lstudent
{
    struct student data;
    struct Lcourse *chead;
    struct Lstudent *next;
};
void load(struct Lstudent** head, const char* filename) {
    *head = NULL; // 如果没读到任何数据需要返回空链表

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("无法打开文件进行读取!\n");
        return;
    }
 
    // 临时存储学生信息
    Student tempStudent;
    CourseListNode* tempCourseNode = NULL;
    struct Lstudent* currentStudent = NULL;
    struct Lstudent* prevStudent = NULL;
 
    while (1) {
        /* 把你的文件格式改成空格分隔，每行开头用一个字符标记是学生还是课程
         * 为什么非要把简单的问题复杂化，给自己增加解析难度呢
         * S 33 yu 15 男 0.00 0.00
           C 3 英语 4.00 150.00
           C 2 数学 3.00 140.00
           C 1 语文 2.00 120.00
           S 123 he 15 男 0.00 0.00
           ...
         */
        char scmark;
        fscanf(fp, " %c", &scmark);
        if (scmark == 'S') {
            // 学生
            int readCount = fscanf(fp, "%d%s%d%s%f%f",
                &tempStudent.xuehao, tempStudent.name, &tempStudent.Class, tempStudent.sex,
                &tempStudent.ZGPA, &tempStudent.KGPA);
            // 抵达文件尾或无效数据
            if (readCount != 6)
                break;
            // 创建新的学生节点
            currentStudent = (struct Lstudent*) malloc(sizeof(struct Lstudent));
            if (currentStudent == NULL) {
                printf("内存分配失败!\n");
                break;
            }
            currentStudent->data = tempStudent;
            currentStudent->chead = NULL;
            currentStudent->next = NULL;
             
            // 如果这不是第一个学生节点，将其链接到链表末尾
            if (prevStudent != NULL) {
                prevStudent->next = currentStudent;
            } else { // 第一个学生节点，更新链表头
                *head = currentStudent;
            }
            prevStudent = currentStudent;
            
        }
        else if (scmark == 'C') {
            // 课程
            // 读取并添加课程信息
            tempCourseNode = (CourseListNode*) malloc(sizeof(CourseListNode));
            if (tempCourseNode == NULL) {
                printf("内存分配失败!\n");
                break;
            }
            int readCount = fscanf(fp, "%d%s%f%f",
                &tempCourseNode->data.daihao, tempCourseNode->data.name,
                &tempCourseNode->data.xuefen, &tempCourseNode->data.chengji);
            // 抵达文件尾或无效数据
            if (readCount != 4) {
                // 需要释放tempCourseNode否则内存泄漏
                free(tempCourseNode);
                break;
            }

            if (currentStudent) {
                tempCourseNode->next = currentStudent->chead;
                currentStudent->chead = tempCourseNode;
            }
        }
        // 既不是S也不是C，非法
        else
            break;
    }
    fclose(fp);
}
 
void freeStudentList(struct Lstudent* head) {
    struct Lstudent* currentStudent = head;
    struct Lstudent* nextStudent;
 
    while (currentStudent != NULL) {
        CourseListNode* courseNode = currentStudent->chead;
        CourseListNode* nextCourseNode;
 
        while (courseNode != NULL) {
            nextCourseNode = courseNode->next;
            free(courseNode);
            courseNode = nextCourseNode;
        }
 
        nextStudent = currentStudent->next;
        free(currentStudent);
        currentStudent = nextStudent;
    }
}
void printStudentListAndCourses(struct Lstudent* head) {
    while (head != NULL) {
        printf("学生信息：学号：%d，姓名：%s，班级：%d，性别：%s，总平均绩点：%f，课程平均绩点：%f\n",
                head->data.xuehao, head->data.name, head->data.Class, head->data.sex,
                head->data.ZGPA, head->data.KGPA);
 
        // 打印课程信息
        CourseListNode* courseNode = head->chead;
        while (courseNode != NULL) {
            printf("课程信息：课程编号：%d，课程名称：%s，课程学分：%.2f，课程成绩：%.2f\n",
                   courseNode->data.daihao, courseNode->data.name,
                   courseNode->data.xuefen, courseNode->data.chengji);
            courseNode = courseNode->next;
        }
 
        head = head->next;
    }
}int main() {
    struct Lstudent* head=NULL;
    // 调用 load 函数加载数据
    load(&head, "信息.txt");
 
    // 确保 load 函数成功读取数据，链表头不为 NULL
    if (head != NULL) {
        // 打印链表信息
        printStudentListAndCourses(head);
 
        // 其他对链表的操作...
 
        // 清理链表，释放内存
        freeStudentList(head);
    } else {
        printf("没有读取到任何学生信息。\n");
    }
 
    return 0;
}
