#include "../inc/meowscript.hpp"

MEOWSCRIPT_MODULE 

class ModuleFilesystem {
public:
    ModuleFilesystem() {
        Module filesystem("filesystem");
        filesystem.add_command(
            {"read",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = tools::check4placeholder(args[0]).to_variable().storage.string;

                    std::string content = read(file);

                    GeneralTypeToken ret;
                    ret.type = General_type::STRING;
                    ret.source.content = content;
                    ret.source.in_quotes = true;
                    return ret;
                }
            }
        ).add_command(
            {"get_line",
                {
                    car_String | car_PlaceHolderAble,
                    car_Number
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file;
                    int line_num = args[0].to_variable().storage.number;
                    if(line_num < 1) {
                        throw errors::MWSMessageException{"Line index must be grater than zero! (" + std::to_string(line_num) + ")",global::get_line()};
                    }
                    file = tools::check4placeholder(args[0]).to_variable().storage.string;

                    std::string content = read(file);

                    std::vector<std::string> lines;
                    std::string tmp;
                    for(size_t i = 0; i < content.size(); ++i) {
                        if(is_newline(i)) {
                            lines.push_back(tmp);
                            tmp = "";
                        }
                        else {
                            tmp += content[i];
                        }

                        if(line_num+1 == lines.size()) {
                            return GeneralTypeToken("\"" + lines[line_num-1] + "\"");
                        }
                    }
                    if(tmp != "") {
                        lines.push_back(tmp);
                    }
                    if(line_num == lines.size()) {
                        return GeneralTypeToken("\"" + lines.back() + "\"");
                    }
                    throw errors::MWSMessageException{"File does not have line: " + std::to_string(line_num),global::get_line()};
                    return general_null;
                }
            }
        ).add_command(
            {"exists",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = tools::check4placeholder(args[0]).to_variable().storage.string;
                    return std::filesystem::exists(file);
                }
            }
        ).add_command(
            {"is_directory",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = tools::check4placeholder(args[0]).to_variable().storage.string;
                    return std::filesystem::is_directory(file);
                }
            }
        ).add_command(
            {"extention",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = tools::check4placeholder(args[0]).to_variable().storage.string;
                    GeneralTypeToken ret;
                    ret.type = General_type::STRING;
                    ret.source.content = std::filesystem::path(file).extension();;
                    ret.source.in_quotes = true;
                    return ret;
                }
            }
        );
        filesystem.enabled = false;
        add_module(filesystem);
    }

}static module_filesystem;