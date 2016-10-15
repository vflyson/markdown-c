#include <stdlib.h>
#include <stdio.h>

#include "../src/markdown.h"

int main(int argc, char** argv[])
{
    char* headerTest = "# hai!             \n"
                        "##       hai!     \n"
                        "### hai!                       \r"
                        "####            hai!           \r\n"
                        "##### hai!        \r"
                        "######        hai!         \n"
                        "#######        hai!         \n\n"
                        "# \n"
                        "## \n"
                        "### \n"
                        "#### \n"
                        "##### \n"
                        "###### \n"
                        "*** \n"
                        "--- \n"
                        "___ \n"
                        "* * * \n"
                        "_     _    _ \n"
                        "--------- \n"
                        "This is a single line paragraph \n\n\n\n"
                        "This is a multiline paragraph\n"
                        "This is a multiline paragraph\n"
                        "This is a multiline paragraph\n\n\n\n"
                        "This is another multiline paragraph\n"
                        "This is another multiline paragraph\n"
                        "This is another multiline paragraph\n\n\n\n"
                        "<> XML escaping <a href=\"#\">XML's not allowed in here</a>\n\n"
                        "Text attributes _italic_, *italic*, __bold__, **bold**, `monospace`.\n\n"
                        "A [link](http://example.com).\n"
    ;

    //printf("%s\n", headerTest);

    md_compile_ast(headerTest);
}
