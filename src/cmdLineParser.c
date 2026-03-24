#include <cmdLineParser.h>
#include <stdio.h>

t_cmdLineParser* initializeCmdLineParser(const char* programName,
                                         const char* description,
                                         const int argc,
                                         const char** argv,
                                         t_ping* ping) {
  t_cmdLineParser* cmdLineParser = galloc(sizeof(t_cmdLineParser), ping);
  cmdLineParser->_programName = programName;
  cmdLineParser->_description = description;
  cmdLineParser->_argc = argc;
  cmdLineParser->_argv = argv;
  cmdLineParser->_optionArgs = galloc(sizeof(t_optionArg*), ping);
  *(cmdLineParser->_optionArgs) = NULL;

  return cmdLineParser;
}

void addOptionArg(t_cmdLineParser* cmdLineParser, t_optionArg optionArg, t_ping* ping) {
  t_optionArg* newOption = galloc(sizeof(t_optionArg), ping);
  *newOption = optionArg;
  listPushFront(cmdLineParser->_optionArgs, listNewElem(newOption, ping), ping);
}

t_optionArg createOption(const char shortName,
                         const char* longName,
                         const char* value,
                         const char* defaultValue,
                         const e_type type,
                         const bool isRequired,
                         const char* description) {
  t_optionArg option;
  option._shortName = shortName;
  option._longName = longName;
  option._value = value;
  option._defaultValue = defaultValue;
  option._type = type;
  option._isRequired = isRequired;
  option._description = description;
  return option;
}

void getStrUsage(char* outBuffer, t_cmdLineParser* cmdLineParser) {
  int offset = 0;
  offset += sprintf(outBuffer + offset, "Usage:\n");
  offset += sprintf(outBuffer + offset, "\t./%s [OPTIONS] destination\n\n", cmdLineParser->_programName);

  offset += sprintf(outBuffer + offset, "Description:\n\t%s", cmdLineParser->_description);

  t_list* elem = *(cmdLineParser->_optionArgs);
  if (elem) {
    offset += sprintf(outBuffer + offset, "\n\nOptions:\n");
  }
  while (elem) {
    t_optionArg option = *((t_optionArg*)elem->data);
    if (option._shortName) {
      offset += sprintf(outBuffer + offset, "  -%c, ", option._shortName);
    } else {
      offset += sprintf(outBuffer + offset, "      ");
    }
    offset += sprintf(outBuffer + offset, "--%s", option._longName);
    offset += sprintf(outBuffer + offset, "\t\t%s", option._description);
    if (option._isRequired) {
      offset += sprintf(outBuffer + offset, "\t\tREQUIRED");
    }
    elem = elem->next;
  }
}
