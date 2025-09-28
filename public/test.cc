#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

static const std::string base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_decode(const std::string &encoded) {
    std::string ret;
    std::vector<int> vec(256, -1);
    
    for (int i = 0; i < 64; i++)
        vec[base64_chars[i]] = i;
    
    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (vec[c] == -1) break;
        val = (val << 6) + vec[c];
        valb += 6;
        if (valb >= 0) {
            ret.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return ret;
}

int main()
{
    int fd = open("123.mp3", O_RDONLY);
    if (fd < 0)
        return -1;

    char buf[325249];
    read(fd, buf, 325249);
    close(fd);

    std::string res = base64_decode(buf);

    int fd2 = open("res.mp3", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd2 < 0)
        return -1;

    write(fd2, &res[0], 325249);
    close(fd2);

    return 0;
}