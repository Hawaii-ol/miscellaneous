#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _MSC_VER
#define bswap16 _byteswap_ushort
#define bswap32 _byteswap_ulong
#elif __GNUC__
#define bswap16 __builtin_bswap16
#define bswap32 __builtin_bswap32
#pragma GCC diagnostic ignored "-Wmultichar"
#else
#error "Unsupported compiler"
#endif

#define ID3VER_2_3 3
#define ID3VER_2_4 4

#define ID3TAG_Picture 'APIC'
#define ID3TAG_Comment 'COMM'
#define ID3TAG_Album 'TALB'
#define ID3TAG_Composer 'TCOM'
#define ID3TAG_Genre 'TCON'
#define ID3TAG_Lyricist 'TEXT'
#define ID3TAG_Title 'TIT2'
#define ID3TAG_OriginalArtist 'TOPE'
#define ID3TAG_Artist 'TPE1'
#define ID3TAG_Track 'TRCK'
#define ID3TAG_Year 'TYER'

#define ENCODING_ISO_8859_1 0x00
#define ENCODING_UTF16BE_BOM 0x01
#define ENCODING_UTF16BE 0x02
#define ENCODING_UTF8 0x03

#define MIME_JPEG "image/jpeg"
#define MIME_PNG "image/png"

static const char *pictypemap[] = {
    "Other",
    "32x32 pixels 'file icon' (PNG only)",
    "Other file icon",
    "Cover (front)",
    "Cover (back)",
    "Leaflet page",
    "Media (e.g. label side of CD)",
    "Lead artist/lead performer/soloist",
    "Artist/performer",
    "Conductor",
    "Band/Orchestra",
    "Composer",
    "Lyricist/text writer",
    "Recording Location",
    "During recording",
    "During performance",
    "Movie/video screen capture",
    "A bright coloured fish",
    "Illustration",
    "Band/artist logotype",
    "Publisher/Studio logotype",
};

struct id3_header
{
    char magic[3];  // 文件头，必须为ID3
    char ver;       // 版本号，3=v2.3，4=v2.4
    char rev;       // 副版本号，为0
    char flags;     // 标识位，只使用高4位
    char size[4];   // 标签长度，每个字节只使用低7位，拼接得到28位长度
};

struct id3_hflags
{
    unsigned unsyrchronised : 1;
    unsigned extended_header : 1;
    unsigned experimental_indicator : 1;
    unsigned footer_present : 1;
};

struct id3_exheader_v2_3
{
    char size[4];
    char flags[2];
    char padding[4];
};

struct id3_exheader_v2_4
{
    char size[4];
    char nbflag;
    char flags;
};

struct id3_frame
{
    char type[4];   // ID3帧属性标识
    char size[4];   // 帧长度
    char flags[2];  // 标识位
};

struct id3_fflags
{
    unsigned tag_alter_discard : 1;
    unsigned file_alter_discard : 1;
    unsigned read_only : 1;
    unsigned group_information : 1;
    unsigned compressed : 1;
    unsigned encrypted : 1;
    unsigned unsyrchronised : 1;
    unsigned data_length_indicator : 1;
};

/* 将32位的Big Endian int转换为本地int */
static inline int id3_int32be_to_int(const char int32be[4])
{
    int i;
    memcpy(&i, int32be, 4);
    return bswap32(i);
}

/* 将4个7位的synchsafe int转换为本地int */
static inline int id3_synchsafe_to_int(const char synchsafe[4])
{
    return (synchsafe[0] << 21) | (synchsafe[1] << 14) | (synchsafe[2] << 7) | synchsafe[3];
}

/* 将header size转换为int */
static inline int id3_header_size(const struct id3_header *header)
{
    return id3_synchsafe_to_int(header->size);
}

/* 将frame size转换为int */
static inline int id3_frame_size(char id3ver, const struct id3_frame *frame)
{
    /* 文档有坑！
     * v2.3的frame size是32位Big Endian int
     * 而v2.4的frame size和header size一样是4个7位的synchsafe int
     */
    if (id3ver == ID3VER_2_3)
        return id3_int32be_to_int(frame->size);
    else if (id3ver == ID3VER_2_4)
        return id3_synchsafe_to_int(frame->size);
    else
        return 0; 
}

/* 校验ID3标签头 */
int id3_validate_header(const struct id3_header *header)
{
    return (
        header->magic[0] == 0x49 &&
        header->magic[1] == 0x44 &&
        header->magic[2] == 0x33 &&
        (header->ver == 3 || header->ver == 4) &&
        header->rev == 0 &&
        header->size[0] < 0x80 &&
        header->size[1] < 0x80 &&
        header->size[2] < 0x80 &&
        header->size[3] < 0x80
    );
}

/* 解析header flags */
struct id3_hflags id3_header_flags(const struct id3_header *header)
{
    struct id3_hflags flags = {0};
    flags.unsyrchronised = !!(header->flags & 0x80);
    flags.extended_header = !!(header->flags & 0x40);
    flags.experimental_indicator = !!(header->flags & 0x20);
    flags.footer_present = !!(header->flags & 0x10);
    return flags;
}

/* 解析ID3帧属性 */
static inline int id3_frame_type(const struct id3_frame *frame)
{
    int id;
    memcpy(&id, frame->type, 4);
    return bswap32(id);
}

/* 解析frame flags */
struct id3_fflags id3_frame_flags(char id3ver, const struct id3_frame *frame)
{
    struct id3_fflags flags = {0};
    flags.tag_alter_discard = !!(frame->flags[0] & 0x40);
    flags.file_alter_discard = !!(frame->flags[0] & 0x20);
    flags.read_only = !!(frame->flags[0] & 0x10);
    if (id3ver == ID3VER_2_3)
    {
        flags.compressed = !!(frame->flags[1] & 0x80);
        flags.encrypted = !!(frame->flags[1] & 0x40);
        flags.group_information = !!(frame->flags[1] & 0x20);
    }
    else if (id3ver == ID3VER_2_4)
    {
        flags.group_information = !!(frame->flags[1] & 0x40);
        flags.compressed = !!(frame->flags[1] & 0x8);
        flags.encrypted = !!(frame->flags[1] & 0x4);
        flags.unsyrchronised = !!(frame->flags[1] & 0x2);
        flags.data_length_indicator = !!(frame->flags[1] & 0x1);
    }
    return flags;
}

/* 计算frame text的字节数
 * 读取到'\0'或L'\0'时停止，字节数包括'\0'或L'\0' */
int frame_text_size(char encoding, const char *text)
{
    int size = 0;
    switch (encoding)
    {
    case ENCODING_ISO_8859_1:
    case ENCODING_UTF8:
        size = (int)strlen(text) + 1;
        break;
    case ENCODING_UTF16BE_BOM:
        // 跳过2字节的BOM
        text += 2;
    case ENCODING_UTF16BE:
        while (text[size] != '\0' && text[size + 1] != '\0')
        {
            size += 2;
        }
        size += 2;
        break;
    }
    return size;
}

/* 将frame text转换为本地字符串
 * 返回的字符串在下次调用此函数前有效
 * 通常，"本地字符串"在简体中文Windows中指GBK字符串，在Unix中指UTF-8字符串。
 * size为text正文的字节数，包括可能的结束标记'\0'或L'\0'，不包括frame首字节的encoding。
 * 如果size=-1，会读取到'\0'或L'\0'为止。 */
char *frame_text_to_locale(char encoding, const char *text, int size)
{
#ifdef _WIN32
    static char *ansi = NULL;
    wchar_t *utf16;
    int len, len16;

    switch (encoding)
    {
    case ENCODING_ISO_8859_1:
        len16 = MultiByteToWideChar(28591, 0, text, size, NULL, 0);
        utf16 = (wchar_t*)malloc(len16 * sizeof(wchar_t));
        MultiByteToWideChar(28591, 0, text, size, utf16, len16);
        break;
    case ENCODING_UTF16BE_BOM:
        // 跳过2字节的BOM
        text += 2;
        if (size != -1)
            size -= 2;
    case ENCODING_UTF16BE:
        if (size == -1)
            size = frame_text_size(ENCODING_UTF16BE, text);
        // 确保wchar_t内存对齐
        utf16 = (wchar_t*)malloc(size);
        memcpy(utf16, text, size);
        len16 = size / sizeof(wchar_t);
        // 转换字节序
        for (wchar_t *cur = utf16; cur < utf16 + len16; cur++)
        {
            *cur = bswap16(*cur);
        }
        break;
    case ENCODING_UTF8:
        len16 = MultiByteToWideChar(CP_UTF8, 0, text, size, NULL, 0);
        utf16 = (wchar_t*)malloc(len16 * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, text, size, utf16, len16);
        break;
    }
    len = WideCharToMultiByte(CP_ACP, 0, utf16, len16, NULL, 0, NULL, FALSE);
    ansi = (char*)realloc(ansi, len + 1);
    WideCharToMultiByte(CP_ACP, 0, utf16, len16, ansi, len, NULL, FALSE);
    ansi[len] = '\0';
    free(utf16);
    return ansi;
#endif
}

/* 解析text frame（frame type以T开头），输出text正文
 * prompt是显示在前面的提示信息 */
static inline void parse_text_frame(const char *prompt, const char *data, int size)
{
    printf("%s: %s\n", prompt, frame_text_to_locale(data[0], data + 1, size - 1));
}

/* 解析APIC frame，输出图片信息并提取图片文件 */
void parse_extract_apic(const char *data, int size)
{
    char encoding;
    const char *mime_type;
    int mime_size;
    char pic_type;
    const char *desc;
    int desc_size;
    char filename[256];
    const char *ext;
    FILE *fp;
    int offset;
    static int picnum;

    picnum++;
    printf("图片%02d元数据\n", picnum);
    encoding = data[0];
    offset = 1;
    // MIME类型
    mime_size = frame_text_size(encoding, data + 1);
    mime_type = frame_text_to_locale(encoding, data + 1, mime_size);
    printf("  MIME类型: %s\n", mime_type);
    if (strcmp(mime_type, MIME_JPEG) == 0)
        ext = "jpg";
    else if (strcmp(mime_type, MIME_PNG) == 0)
        ext = "png";
    else
        ext = NULL;
    offset += mime_size;
    // 图片类型
    pic_type = data[offset];
    if (pic_type >= 0 && pic_type < sizeof(pictypemap))
        printf("  图片类型: %s\n", pictypemap[pic_type]);
    else
        printf("  图片类型: 未知\n");
    offset++;
    // 图片描述
    desc_size = frame_text_size(encoding, data + offset);
    desc = frame_text_to_locale(encoding, data + offset, desc_size);
    printf("  图片描述: %s\n", desc);
    offset += desc_size;
    // 提取图片文件
    printf("  图片大小: %d字节\n", size - offset);
    if (ext != NULL)
    {
        sprintf(filename, "id3pic%02d.%s", picnum, ext);
        if ((fp = fopen(filename, "wb")) == NULL)
        {
            printf("  导出图片%s失败: %s\n", filename, strerror(errno));
        }
        else
        {
            printf("  导出图片%s至当前目录...\n", filename);
            fwrite(data + offset, 1, size - offset, fp);
            fclose(fp);
        }
    }
    else
    {
        printf("  无法导出图片%02d: 不支持的MIME格式\n", picnum);
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;
    struct id3_header header;
    struct id3_hflags hflags;
    struct id3_frame frame;
    struct id3_fflags fflags;
    int tag_size, frame_size;
    int offset = 0;
    char buf[256], *data;

    if (argc < 2)
    {
        printf("用法: %s mp3file.mp3\n", argv[0]);
        printf("  解析mp3文件标签，并导出其中的图片。\n");
        return 0;
    }
    if ((fp = fopen(argv[1], "rb")) == NULL)
    {
        perror(argv[1]);
        return 0;
    }
    printf("文件名: %s\n", argv[1]);

    // header
    if (fread(&header, sizeof(header), 1, fp) == 0 || !id3_validate_header(&header))
    {
        printf("未找到ID3标签，文件可能未包含相关信息或格式无效。\n");
        return 0;
    }
    tag_size = id3_header_size(&header);
    hflags = id3_header_flags(&header);
    // extended header
    if (hflags.extended_header)
    {
        if (header.ver == ID3VER_2_3)
        {
            struct id3_exheader_v2_3 exheader;
            int crc32size, padding;
            if (fread(&exheader, sizeof(exheader), 1, fp) == 0)
            {
                printf("读取数据时出错。\n");
                return 1;
            }
            // 跳过crc32，如果有的话
            crc32size = id3_int32be_to_int(exheader.size) + 4 - sizeof(exheader);
            fseek(fp, crc32size, SEEK_CUR);
            offset += sizeof(exheader) + crc32size;
            // tag_size减掉尾部的padding
            padding = id3_int32be_to_int(exheader.padding);
            tag_size -= padding;
        }
        else if (header.ver == ID3VER_2_4)
        {
            struct id3_exheader_v2_4 exheader;
            int size;
            if (fread(&exheader, sizeof(exheader), 1, fp) == 0)
            {
                printf("读取数据时出错。\n");
                return 1;
            }
            // 跳过整个extended header
            size = id3_synchsafe_to_int(exheader.size);
            fseek(fp, size - sizeof(exheader), SEEK_CUR);
            offset += size;
        }
    }
    // frames
    offset = 0;
    while (offset < tag_size)
    {
        if (fread(&frame, sizeof(frame), 1, fp) == 0)
        {
            printf("读取数据时出错。\n");
            return 1;
        }
        frame_size = id3_frame_size(header.ver, &frame);
        fflags = id3_frame_flags(header.ver, &frame);
        if (fflags.compressed || fflags.encrypted)
        {
            printf("frame %4s被压缩或加密，跳过。\n", frame.type);
            fseek(fp, frame_size, SEEK_CUR);
            offset += sizeof(frame) + frame_size;
            continue;
        }
        if (frame_size > sizeof(buf))
            data = (char*)malloc(frame_size);
        else
            data = buf;
        if (fread(data, 1, frame_size, fp) < frame_size)
        {
            printf("读取数据时出错。\n");
            return 1;
        }
        switch (id3_frame_type(&frame))
        {
        case ID3TAG_Album:
            parse_text_frame("专辑", data, frame_size);
            break;
        case ID3TAG_Composer:
            parse_text_frame("作曲", data, frame_size);
            break;
        case ID3TAG_Genre:
            parse_text_frame("类型", data, frame_size);
            break;
        case ID3TAG_Lyricist:
            parse_text_frame("作词", data, frame_size);
            break;
        case ID3TAG_Title:
            parse_text_frame("标题", data, frame_size);
            break;
        case ID3TAG_OriginalArtist:
            parse_text_frame("原创艺术家", data, frame_size);
            break;
        case ID3TAG_Artist:
            parse_text_frame("艺术家", data, frame_size);
            break;
        case ID3TAG_Track:
            parse_text_frame("音轨", data, frame_size);
            break;
        case ID3TAG_Year:
            parse_text_frame("年份", data, frame_size);
            break;
        case ID3TAG_Picture:
            parse_extract_apic(data, frame_size);
            break;
        default:
            // printf("尚未支持的frame: %4s，跳过。\n", frame.type);
            break;
        }

        if (data != buf)
            free(data);
        offset += sizeof(frame) + frame_size;
    }
    fclose(fp);
    return 0;
}
