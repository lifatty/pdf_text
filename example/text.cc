#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"

jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

static const int PAGE_WIDTH = 420;
static const int PAGE_HEIGHT = 400;
static const int CELL_WIDTH = 20;
static const int CELL_HEIGHT = 20;
static const int CELL_HEADER = 10;





void draw_fonts (HPDF_Page   page)
{
   int i;
    int j;

    HPDF_Page_BeginText (page);
    for (int i = 1; i <= 20; ++i) { // 20 行
        for (int j = 1; j <= 10; ++j) { // 每行 10 个字符
            unsigned char buf[2];
            int y = PAGE_HEIGHT - 55 - ((i - 1) * CELL_HEIGHT);
            int x = j * CELL_WIDTH + 50;
            buf[1] = 0x00;

            buf[0] = (i - 1) * 10 + (j - 1) + 32; // 将 ASCII 码转换为字符

            if (buf[0] <= 255) { // 确保字符在合法范围内
                double d = x - HPDF_Page_TextWidth(page, (char*)buf) / 2;
                HPDF_Page_TextOut(page, d, y, (char*)buf);
            }
        }
    }

    HPDF_Page_EndText (page);
}


int main (int argc, char **argv)
{
    HPDF_Doc  pdf;
    char fname[256];
    HPDF_Font font;
    const char *font_name;
    int i = 0;
    HPDF_Outline root;

    const char *encodings[] = {
            "Arial-Unicode-MS",
            NULL
    };

    pdf = HPDF_NewEx (error_handler, NULL, NULL, 0, NULL);
    if (!pdf) {
        printf ("error: cannot create PdfDoc object\n");
        return 1;
    }

    if (setjmp(env)) {
        HPDF_Free (pdf);
        return 1;
    }

    strcpy (fname, argv[0]);
    strcat (fname, ".pdf");

    HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    font_name = HPDF_LoadTTFontFromFile (pdf, "../text/arial-unicode/Arial-Unicode-MS.ttf", HPDF_TRUE);
    font = HPDF_GetFont (pdf, font_name, NULL); 

    while (encodings[i]) {
        HPDF_Page page = HPDF_AddPage (pdf);
        HPDF_Font font2;

        HPDF_Page_SetWidth (page, PAGE_WIDTH);
        HPDF_Page_SetHeight (page, PAGE_HEIGHT);
        HPDF_Page_BeginText (page);
        HPDF_Page_SetFontAndSize (page, font, 20);
        HPDF_Page_MoveTextPos (page, 40, PAGE_HEIGHT - 40);
        HPDF_Page_ShowText (page, encodings[i]);
        HPDF_Page_ShowText (page, " Encoding");
        HPDF_Page_EndText (page);

        font2 = HPDF_GetFont (pdf, font_name, NULL);

        HPDF_Page_SetFontAndSize (page, font2, 14);
        draw_fonts (page);

        i++;
    }

    /* save the document to a file */
    HPDF_SaveToFile (pdf, fname);

    /* clean up */
    HPDF_Free (pdf);

    return 0;
}