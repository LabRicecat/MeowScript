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
                    std::string file = global::include_path.top().parent_path().string() + MEOWSCRIPT_DIR_SL + tools::check4placeholder(args[0]).to_variable().storage.string.content;
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
                    file = global::include_path.top().parent_path().string() + MEOWSCRIPT_DIR_SL + tools::check4placeholder(args[0]).to_variable().storage.string.content;

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
                    std::string file = global::include_path.top().parent_path().string() + MEOWSCRIPT_DIR_SL + tools::check4placeholder(args[0]).to_variable().storage.string.content;
                    return std::filesystem::exists(file);
                }
            }
        ).add_command(
            {"is_directory",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = global::include_path.top().parent_path().string() + MEOWSCRIPT_DIR_SL + tools::check4placeholder(args[0]).to_variable().storage.string.content;
                    return std::filesystem::is_directory(file);
                }
            }
        ).add_command(
            {"extention",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = global::include_path.top().parent_path().string() + MEOWSCRIPT_DIR_SL + tools::check4placeholder(args[0]).to_variable().storage.string.content;
                    GeneralTypeToken ret;
                    ret.type = General_type::STRING;
                    ret.source.content = std::filesystem::path(file).extension().string();
                    ret.source.in_quotes = true;
                    return ret;
                }
            }
        ).add_command(
            {"write",
                {
                    car_String | car_PlaceHolderAble,
                    car_String | car_PlaceHolderAble,
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = global::include_path.top().parent_path().string() + MEOWSCRIPT_DIR_SL + tools::check4placeholder(args[0]).to_variable().storage.string.content;
                    std::string to_write = tools::check4placeholder(args[0]).to_variable().storage.string.content;

                    std::ofstream off(file,std::ios::trunc);
                    off.close();
                    off.open(file,std::ios::app);
                    off << to_write;
                    off.close();
                    return general_null;
                }
            }
        ).add_command(
            {"append",
                {
                    car_String | car_PlaceHolderAble,
                    car_String | car_PlaceHolderAble,
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    std::string file = global::include_path.top().parent_path().string() + MEOWSCRIPT_DIR_SL + tools::check4placeholder(args[0]).to_variable().storage.string.content;
                    std::string to_write = tools::check4placeholder(args[0]).to_variable().storage.string.content;

                    std::ofstream off(file,std::ios::app);
                    off << to_write;
                    off.close();
                    return general_null;
                }
            }
        );
        filesystem.enabled = false;
        add_module(filesystem);
    }

}static module_filesystem;