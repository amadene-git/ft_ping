#ifndef CMD_LINE_PARSER_H
#define CMD_LINE_PARSER_H

#include <list.h>
#include <stdbool.h>

typedef enum e_type { STRING, ULONG } e_type;

typedef struct s_optionArg {
  char _shortName;
  const char* _longName;
  const char* _value;
  const char* _defaultValue;
  e_type _type;
  bool _isRequired;
  const char* _description;
} t_optionArg;

typedef struct s_cmdLineParser {
  const char* _programName;
  const char* _description;
  int _argc;
  const char** _argv;
  t_list** _optionArgs;
} t_cmdLineParser;

t_cmdLineParser* initializeCmdLineParser(const char* programName,
                                         const char* description,
                                         const int argc,
                                         const char** argv,
                                         t_ping* ping);
void addOptionArg(t_cmdLineParser* cmdLineParser, t_optionArg optionArg, t_ping* ping);
t_optionArg createOption(const char shortName,
                         const char* longName,
                         const char* value,
                         const char* defaultValue,
                         const e_type type,
                         const bool isRequired,
                         const char* description);
void getStrUsage(char* outBuffer, t_cmdLineParser* cmdLineParser);

#endif