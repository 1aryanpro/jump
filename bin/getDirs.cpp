#include <array>
#include <cstdio>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

vector<string> exec(const char *cmd) {
  int count = 0;
  vector<string> result;

  unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

  array<char, 128> buffer;
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result.push_back(buffer.data());
    count++;
  }

  return result;
}

bool fzf(string src, string cmp) {
  int srcPos = 0;
  int cmpPos = 0;

  while (srcPos < src.length() && cmpPos < cmp.length()) {
    if (src[srcPos] == cmp[cmpPos]) {
      cmpPos++;
    }
    srcPos++;
  }

  return (cmpPos >= cmp.length());
}

int main(int argc, char **argv) {

  vector<string> output = exec("find ~/.config ~/Documents/Github ~/Desktop "
                               "-type d -maxdepth 1 -mindepth 0");
  int count = output.size();
  string dirs[count];
  string paths[count];
  copy(output.begin(), output.end(), paths);

  regex basename(".+/(.+)\\s");
  for (int i = 0; i < count; i++) {
    dirs[i] = regex_replace(paths[i], basename, "$1");
    transform(dirs[i].begin(), dirs[i].end(), dirs[i].begin(), ::tolower);
  }

  string search = argv[1];
  string results[count];
  int resSize = 0;
  transform(search.begin(), search.end(), search.begin(), ::tolower);

  for (int i = 0; i < count; i++) {
    if (fzf(dirs[i], search)) {
      results[resSize] = paths[i];
      resSize++;
    }
  }

  if (resSize == 1) {
    char *cmd;
    int len = asprintf(&cmd, "echo '%s' > ~/scripts/jump/store/jumpDir.txt",
                       results[0].c_str());
    system(cmd);
    return 0;
  }
  
  system("echo '.' > ~/scripts/jump/store/jumpDir.txt");

  if (resSize == 0) {
    cout << "No results." << endl;
    return 0;
  }

  for (int i = 0; i < resSize; i++) {
    cout << results[i];
  }

  return 0;
}
