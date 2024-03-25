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
 
 
        // 读取学生信息
        int readCount = fscanf(fp, "%d,%[^,],%d,%[^,],%f,%f\n",
                             &tempStudent.xuehao, tempStudent.name, &tempStudent.Class, tempStudent.sex,
                             &tempStudent.ZGPA, &tempStudent.KGPA);
        if (readCount != 6) { // 没读取到完整的学生信息，可能是到了课程信息部分，或文件格式错误
            break;
        }
 
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
 
        // 读取并添加课程信息
        while (1) {
            tempCourseNode = (CourseListNode*) malloc(sizeof(CourseListNode));
            if (tempCourseNode == NULL) {
                printf("内存分配失败!\n");
                break;
            }
            readCount = fscanf(fp, "%d,%[^,],%f,%f\n",
                              &tempCourseNode->data.daihao, tempCourseNode->data.name,
                              &tempCourseNode->data.xuefen, &tempCourseNode->data.chengji);
            if (readCount != 4||feof(fp)) { // 没读取到完整的课程信息，可能是到了下一个学生信息，或文件格式错误
                break;
            }
 
            tempCourseNode->next = currentStudent->chead;
            currentStudent->chead = tempCourseNode;
 
        }
 
      if (feof(fp)) {
                break; // 到达文件末尾，跳出内层循环
            }
     printf("1");
 
    }
    printf("2");
    int result = fclose(fp);
    if (result != 0) {
        perror("fclose failed");
    } else {
        printf("文件读取成功并关闭\n");
    }
 
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
