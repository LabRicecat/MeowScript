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
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;
                    if(!fs::exists(file)) {
                        throw errors::MWSMessageException{"File does not exist! (" + file + ")",global::get_line()};
                    }
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
                    car_Number | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;
                    if(!fs::exists(file)) {
                        throw errors::MWSMessageException{"File does not exist! (" + file + ")",global::get_line()};
                    }
                    auto line_n = tools::check4placeholder(args[1]);
                    if(line_n.type != General_type::NUMBER) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: Number\n\t- But got: " + general_t2token(line_n.type).content,global::get_line()};
                    }
                    int line_num = line_n.to_variable().storage.number;
                    if(line_num < 1) {
                        throw errors::MWSMessageException{"Line index must be grater than zero! (" + std::to_string(line_num) + ")",global::get_line()};
                    }
                    std::string content = read(file);

                    std::vector<std::string> lines;
                    std::string tmp;
                    for(auto i : content) {
                        if(i == '\n') {
                            lines.push_back(tmp);
                            tmp = "";
                        }
                        else {
                            tmp += i;
                        }
                    }
                    if(tmp != "") {
                        lines.push_back(tmp);
                    }
                    if(line_num > lines.size()) {
                        throw errors::MWSMessageException{"File does not have line: " + std::to_string(line_num),global::get_line()};
                    }
                    return lines[line_num-1];
                }
            }
        ).add_command(
            {"exists",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;
                    if(!fs::exists(file)) {
                        throw errors::MWSMessageException{"File does not exist! (" + file + ")",global::get_line()};
                    }
                    return fs::exists(file);
                }
            }
        ).add_command(
            {"is_directory",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;
                    if(!fs::exists(file)) {
                        throw errors::MWSMessageException{"File does not exist! (" + file + ")",global::get_line()};
                    }

                    return fs::is_directory(file);
                }
            }
        ).add_command(
            {"extention",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;

                    GeneralTypeToken ret;
                    ret.type = General_type::STRING;
                    ret.source.content = fs::path(file).extension().string();
                    ret.source.in_quotes = true;
                    return ret;
                }
            }
        ).add_command(
            {"filename",
                {
                    car_String | car_PlaceHolderAble
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;

                    GeneralTypeToken ret;
                    ret.type = General_type::STRING;
                    ret.source.content = fs::path(file).filename().string();
                    ret.source.in_quotes = true;
                    return ret;
                }
            }
        ).add_command(
            {"write",
                {
                    car_String | car_PlaceHolderAble,
                    car_String | car_Number | car_List | car_PlaceHolderAble,
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;
                    Variable to_write = tools::check4placeholder(args[1]).to_variable();
                    if(to_write.type != Variable::Type::String && to_write.type != Variable::Type::Number && to_write.type != Variable::Type::List) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: [String,Number,List]\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::ofstream off(file,std::ios::trunc);
                    off.close();
                    off.open(file,std::ios::app);
                    if(to_write.type == Variable::Type::String) {
                        off << to_write.storage.string.content;
                    }
                    else {
                        off << to_write.to_string();
                    }
                    off.close();
                    return general_null;
                }
            }
        ).add_command(
            {"append",
                {
                    car_String | car_PlaceHolderAble,
                    car_String | car_Number | car_List | car_PlaceHolderAble,
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;
                    Variable to_write = tools::check4placeholder(args[1]).to_variable();
                    if(to_write.type != Variable::Type::String && to_write.type != Variable::Type::Number && to_write.type != Variable::Type::List) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: [String,Number,List]\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::ofstream off(file,std::ios::app);
                    if(to_write.type == Variable::Type::String) {
                        off << to_write.storage.string.content;
                    }
                    else {
                        off << to_write.to_string();
                    }
                    off.close();
                    return general_null;
                }
            }
        ).add_command(
            {"size",
                {
                    car_String | car_PlaceHolderAble,
                },
                [](std::vector<GeneralTypeToken> args)->GeneralTypeToken {
                    auto fname = tools::check4placeholder(args[0]);
                    if(fname.type != General_type::STRING) {
                        throw errors::MWSMessageException{"Invalid argument!\n\t- Expected: String\n\t- But got: " + general_t2token(fname.type).content,global::get_line()};
                    }
                    std::string file = global::include_parent_path().string() + fname.source.content;
                    if(!fs::exists(file)) {
                        throw errors::MWSMessageException{"File does not exist! (" + file + ")",global::get_line()};
                    }

                    return fs::file_size(file);
                }
            }
        );
        filesystem.enabled = false;
        add_module(filesystem);
    }

}static module_filesystem;