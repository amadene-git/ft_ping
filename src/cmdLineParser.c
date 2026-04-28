#include <cmdLineParser.h>
#include <ft_ping.h>
#include <stdio.h>

void initializeCmdLineParser(const char* programName,
                             const char* description,
                             const int argc,
                             const char** argv,
                             t_ping* ping) {
  ping->cmdLineParser = galloc(sizeof(t_cmdLineParser), ping);
  ping->cmdLineParser->_programName = programName;
  ping->cmdLineParser->_description = description;
  ping->cmdLineParser->_argc = argc;
  ping->cmdLineParser->_argv = argv;
  ping->cmdLineParser->_optionArgs = galloc(sizeof(t_optionArg*), ping);
  *(ping->cmdLineParser->_optionArgs) = NULL;
}

void addOptionArg(t_optionArg optionArg, t_ping* ping) {
  t_optionArg* newOption = galloc(sizeof(t_optionArg), ping);
  *newOption = optionArg;
  listPushFront(ping->cmdLineParser->_optionArgs, listNewElem(newOption, ping), ping);
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

void getStrHelp(char* outBuffer, t_cmdLineParser* cmdLineParser) {
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
    offset += sprintf(outBuffer + offset, "--%-10s", option._longName);
    offset += sprintf(outBuffer + offset, "\t\t%s", option._description);
    if (option._isRequired) {
      offset += sprintf(outBuffer + offset, "\t\tREQUIRED");
    }
    elem = elem->next;
    offset += sprintf(outBuffer + offset, "\n");
  }
}

const char* getOptionValue(const char* optionName, t_ping* ping) {
  t_list* elem = *ping->cmdLineParser->_optionArgs;
  while (elem) {
    if (strcmp(((t_optionArg*)elem->data)->_longName, optionName) == 0) {
      return ((t_optionArg*)elem->data)->_value;
    }
    elem = elem->next;
  }
  return NULL;
}
