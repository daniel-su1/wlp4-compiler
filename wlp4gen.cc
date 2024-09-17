#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;

class ParseNode {
   public:
    string symbol;  // e.g. NUM
    string lexeme;  // terminal symbol e.g. 42
    vector<shared_ptr<ParseNode>> children;
    string type;  // e.g. int

    ParseNode(const string& s) : symbol(s), lexeme("") {}
};

struct ProcedureSignature {
    vector<string> parameterTypes;
    vector<string> parameterNames;
    string returnType;
    shared_ptr<ParseNode> returnExpression;
    shared_ptr<ParseNode> statements;
};

std::unordered_map<std::string, ProcedureSignature> procedureSignatures;
struct symbolTable {
    string type;
    string variableType;  // parameter, variable
    string value;
    string initialValue;
    int offset;
};

std::unordered_map<std::string, unordered_map<string, symbolTable>> symbolTables;
std::vector<std::pair<std::string, std::string>> assignmentOrder;
int labelCounter = 0;

string generateLabel() { return "label" + to_string(labelCounter++); }
shared_ptr<ParseNode> root;
vector<string> errorMessages;

void buildTreeHelper(shared_ptr<ParseNode> node, vector<string>& lines) {
    // cout << "current line: " << lines.front() << endl;
    // cout << "node symbol: " << node->symbol << endl;
    // cout << "node lexeme: " << node->lexeme << endl;
    // cout << "children:";
    // for (auto child : node->children) {
    //     cout << child->symbol << " ";
    // }
    // cout << endl;
    if (lines.size() == 0) {
        if (node->lexeme == "NULL") {
            cerr << "ERROR: null" << endl;
        }
        return;
    }
    string line = lines.front();
    vector<string> childrenLexemes;

    istringstream iss(line);
    string token;
    iss >> token;
    string parent = token;
    // cout << "token: " << token << endl;
    if (token != node->symbol && token != node->lexeme) {
        return;
    }

    while (iss >> token) {
        if (token == ":") {
            iss >> token;
            break;
        }
        childrenLexemes.push_back(token);
    }
    // cout << "test" << endl;
    if (childrenLexemes.size() == 1 && node->lexeme != "") {
        if (childrenLexemes.at(0) == node->lexeme && parent != childrenLexemes.at(0)) {
            return;
        }
    }
    if (node->lexeme == childrenLexemes.at(0) && parent == node->symbol) {
        return;
    }
    lines.erase(lines.begin());

    // cout << "childrenLexemes: ";
    // for (string c : childrenLexemes) {
    //     cout << c << " ";
    // }
    // cout << endl;

    if (childrenLexemes.size() == 1) {
        if (node->lexeme == "") {
            // cout << "adding " << childrenLexemes.at(0) << " lexeme to " << node->symbol << endl;
            node->lexeme = childrenLexemes.at(0);
            buildTreeHelper(node, lines);
        } else {
            shared_ptr<ParseNode> child = make_shared<ParseNode>(node->lexeme);
            node->lexeme = "";
            child->lexeme = childrenLexemes.at(0);

            node->children.push_back(child);
            buildTreeHelper(child, lines);
        }
    } else {
        if (node->lexeme != "") {
            shared_ptr<ParseNode> child = make_shared<ParseNode>(node->lexeme);
            node->lexeme = "";
            for (int i = 0; i < childrenLexemes.size(); ++i) {
                child->children.push_back(make_shared<ParseNode>(childrenLexemes.at(i)));
                buildTreeHelper(child->children.at(i), lines);
            }
            node->children.push_back(child);

        } else {
            for (int i = 0; i < childrenLexemes.size(); ++i) {
                node->children.push_back(make_shared<ParseNode>(childrenLexemes.at(i)));
                // cout << "adding " << childrenLexemes.size() << " children to " << node->symbol
                //  << endl;
                buildTreeHelper(node->children.at(i), lines);
            }
        }
    }
    return;
}

void buildTree(vector<string>& lines) {
    root = make_shared<ParseNode>("start");
    buildTreeHelper(root, lines);
}
void printTreeHelper(const shared_ptr<ParseNode>& node, int depth = 0) {
    if (!node) return;

    // Print symbol
    cerr << node->symbol;

    // Print lexeme if it exists
    if (!node->lexeme.empty()) {
        cerr << " " << node->lexeme;
        if (!node->type.empty()) {
            cerr << " : " << node->type;
        }
        cerr << endl;
    } else {
        for (const auto& child : node->children) {
            cerr << " " << child->symbol;  // Print children on the same line
        }
        if (!node->type.empty()) {
            cerr << " : " << node->type;
        }
        cerr << endl;
    }

    // Recursively print children
    for (const auto& child : node->children) {
        printTreeHelper(child, depth + 1);
    }
}

void printTree() {
    if (errorMessages.empty()) {
        printTreeHelper(root);
    } else {
        for (const auto& error : errorMessages) {
            cerr << error << endl;
        }
    }
}
void printProcedureSignatures(
    const std::unordered_map<std::string, ProcedureSignature>& procedureSignatures) {
    for (const auto& [procedureName, signature] : procedureSignatures) {
        cerr << "Procedure: " << procedureName << std::endl;
        cerr << "Return Type: " << signature.returnType << std::endl;
        cerr << "Parameter Types: ";
        for (const auto& paramType : signature.parameterTypes) {
            cerr << paramType << " ";
        }
        cerr << std::endl;
        cerr << "Parameter Names: ";
        for (const auto& paramName : signature.parameterNames) {
            cerr << paramName << " ";
        }
        cerr << std::endl;
        cerr << "Return Expression: ";
        if (signature.returnExpression) {
            printTreeHelper(signature.returnExpression);
        }
        cerr << std::endl;
    }
}
void printSymbolTables() {
    for (const auto& [procedureName, symbolTable] : symbolTables) {
        cerr << "Procedure: " << procedureName << endl;
        for (const auto& [variableName, tableEntry] : symbolTable) {
            cerr << "Variable: " << variableName << endl;
            cerr << "Type: " << tableEntry.type << endl;
            cerr << "Variable Type: " << tableEntry.variableType << endl;
            cerr << "Value: " << tableEntry.value << endl;
            cerr << "Initial Value: " << tableEntry.initialValue << endl;
            cerr << "Offset: " << tableEntry.offset << endl;
        }
        cerr << endl;
    }
}
string getVariableFromLvalue(const shared_ptr<ParseNode>& lvalueNode);
void processArgList(const shared_ptr<ParseNode>& node, vector<string>& argTypes,
                    const string& currentProcedure);

void annotateTypes(const shared_ptr<ParseNode>& node, string currentProcedure) {
    if (node->symbol == "dcl") {
        // Handle declaration nodes

        if (node->children.size() >= 2 && node->children.at(0)->children.size() >= 1 &&
            node->children.at(0)->children.at(0)->symbol == "INT" &&
            node->children.at(0)->children.at(0)->lexeme == "int") {
            if (node->children.at(0)->children.size() == 1) {
                if (symbolTables[currentProcedure].count(node->children.at(1)->lexeme) > 0) {
                    errorMessages.push_back("ERROR: Duplicate entry in symbol table");
                    return;
                }
                node->children.at(1)->type = "int";
                // cout << "currentProcedure: " << currentProcedure << endl;
                symbolTables[currentProcedure][node->children.at(1)->lexeme].type = "int";
            } else if (node->children.at(0)->children.at(1)->symbol == "STAR" &&
                       node->children.at(0)->children.at(1)->lexeme == "*") {
                if (symbolTables[currentProcedure].count(node->children.at(1)->lexeme) > 0) {
                    errorMessages.push_back("ERROR: Duplicate entry in symbol table");
                    return;
                }
                node->children.at(1)->type = "int*";

                symbolTables[currentProcedure][node->children.at(1)->lexeme].type = "int*";
            }
        }
    } else if (node->symbol == "expr") {
        // Handle expression nodes
        if (node->children.size() == 1) {
            if (node->children.at(0)->symbol == "term") {
                annotateTypes(node->children.at(0), currentProcedure);
                node->type = node->children.at(0)->type;
            }
        }
        if (node->children.size() == 3) {
            annotateTypes(node->children.at(0), currentProcedure);
            annotateTypes(node->children.at(2), currentProcedure);
            if (node->children.at(0)->type == "int*" && node->children.at(2)->type == "int*" &&
                node->children.at(1)->lexeme == "+") {
                errorMessages.push_back("ERROR: cannot add two int*");
            }
            if (node->children.at(0)->type != node->children.at(2)->type) {
                node->type = "int*";
            } else {
                node->type = "int";
            }
        }

    } else if (node->symbol == "term") {
        if (node->children.size() == 1) {
            if (node->children.at(0)->symbol == "factor") {
                annotateTypes(node->children.at(0), currentProcedure);
                node->type = node->children.at(0)->type;
            }
        } else if (node->children.size() == 3) {
            annotateTypes(node->children.at(0), currentProcedure);
            annotateTypes(node->children.at(2), currentProcedure);
            if (node->children.at(0)->type != node->children.at(2)->type) {
                errorMessages.push_back("ERROR: Type mismatch in term");
            } else {
                node->type = node->children.at(0)->type;
            }
        }
    } else if (node->symbol == "factor") {
        if (node->children.size() == 1) {
            if (node->children.at(0)->symbol == "ID") {
                if (symbolTables[currentProcedure].count(node->children.at(0)->lexeme) == 0) {
                    errorMessages.push_back("ERROR: Variable " + node->children.at(0)->lexeme +
                                            " not defined");
                    return;
                }
                if (symbolTables[currentProcedure][node->children.at(0)->lexeme].type == "int") {
                    node->type = "int";
                    node->children.at(0)->type = "int";
                } else if (symbolTables[currentProcedure][node->children.at(0)->lexeme].type ==
                           "int*") {
                    node->type = "int*";
                    node->children.at(0)->type = "int*";
                }
            } else if (node->children.at(0)->symbol == "NUM") {
                node->type = "int";
                node->children.at(0)->type = "int";
            } else if (node->children.at(0)->symbol == "NULL") {
                node->type = "int*";
                node->children.at(0)->type = "int*";
            }
        } else if (node->children.size() == 2) {
            if (node->children.at(0)->symbol == "STAR") {
                node->type = "int";
                if (node->children.at(1)->symbol == "factor") {
                    annotateTypes(node->children.at(1), currentProcedure);
                }
                if (node->children.at(1)->type != "int*") {
                    errorMessages.push_back("ERROR: Expected child of type int*");
                }
            } else if (node->children.at(0)->symbol == "AMP") {
                node->type = "int*";
                if (node->children.at(1)->symbol == "lvalue") {
                    annotateTypes(node->children.at(1), currentProcedure);
                }
                if (node->children.at(1)->type != "int") {
                    errorMessages.push_back("ERROR: Expected child of type int");
                }
            }
        } else if (node->children.size() == 3 || node->children.size() == 4) {
            if (node->children.at(0)->symbol == "LPAREN") {
                annotateTypes(node->children.at(1), currentProcedure);
                node->type = node->children.at(1)->type;
            } else if (node->children.at(0)->symbol == "ID") {
                string calledProcedure = node->children.at(0)->lexeme;
                if (procedureSignatures.find(calledProcedure) != procedureSignatures.end()) {
                    node->type = procedureSignatures[calledProcedure].returnType;
                    // cout << "called procedure: " << calledProcedure
                    //      << " return type: " << node->type << endl;

                    // Check arguments
                    vector<string> argTypes;
                    if (node->children.size() == 4) {
                        auto argsNode = node->children.at(2);
                        if (argsNode->symbol == "arglist") {
                            // New function to handle arglist recursion
                            processArgList(argsNode, argTypes, currentProcedure);
                        }
                    }

                    // Compare argument types with signature
                    const auto& expectedTypes = procedureSignatures[calledProcedure].parameterTypes;
                    // cout << "expected types: ";
                    // for (const auto& t : expectedTypes) {
                    //     cout << t << " ";
                    // }
                    // cout << endl;
                    // cout << "arg types: ";
                    // for (const auto& t : argTypes) {
                    //     cout << t << " ";
                    // }
                    // cout << endl;
                    if (argTypes.size() != expectedTypes.size()) {
                        errorMessages.push_back(
                            "ERROR: Incorrect number of arguments for procedure " +
                            calledProcedure);
                    } else {
                        for (size_t i = 0; i < argTypes.size(); ++i) {
                            if (argTypes[i] != expectedTypes[i]) {
                                errorMessages.push_back(
                                    "ERROR: Argument type mismatch for procedure " +
                                    calledProcedure);
                                break;
                            }
                        }
                    }
                } else {
                    errorMessages.push_back("ERROR: Procedure " + calledProcedure + " not defined");
                }
            } else if (node->children.at(0)->symbol == "GETCHAR") {
                node->type = "int";
            }
        }

        else if (node->children.size() == 5) {
            if (node->children.at(3)->symbol == "expr") {
                annotateTypes(node->children.at(3), currentProcedure);
                if (node->children.at(3)->type == "int") {
                    node->type = "int*";
                } else {
                    errorMessages.push_back("ERROR: Expected child of type int");
                }
            }
        }
    } else if (node->symbol == "lvalue") {
        if (node->children.size() == 1) {
            if (node->children.at(0)->symbol == "ID") {
                string varName = node->children.at(0)->lexeme;
                if (symbolTables[currentProcedure].find(varName) !=
                    symbolTables[currentProcedure].end()) {
                    node->type = symbolTables[currentProcedure][varName].type;
                    node->children.at(0)->type = node->type;
                } else {
                    errorMessages.push_back("ERROR: Variable " + varName + " not defined");
                }
            }
        } else if (node->children.size() == 2) {
            if (node->children.at(0)->symbol == "STAR") {
                node->type = "int";
                annotateTypes(node->children.at(1), currentProcedure);
                if (node->children.at(1)->type != "int*") {
                    errorMessages.push_back("ERROR: Expected child of type int*");
                }
            }
        } else if (node->children.size() == 3) {
            if (node->children.at(0)->symbol == "LPAREN") {
                annotateTypes(node->children.at(1), currentProcedure);
                node->type = node->children.at(1)->type;
            }
        }
    } else if (node->symbol == "dcls") {
        if (node->children.size() == 5) {
            annotateTypes(node->children.at(0), currentProcedure);
            annotateTypes(node->children.at(1), currentProcedure);
            if (node->children.at(3)->symbol == "NUM") {
                node->children.at(3)->type = "int";
            } else if (node->children.at(3)->symbol == "NULL") {
                node->children.at(3)->type = "int*";
            }
            if (node->children.at(1)->children.at(1)->type != node->children.at(3)->type) {
                errorMessages.push_back("ERROR: Type mismatch in dcls");
            }
            symbolTables[currentProcedure][node->children.at(1)->children.at(1)->lexeme]
                .variableType = "variable";
            symbolTables[currentProcedure][node->children.at(1)->children.at(1)->lexeme].value =
                node->children.at(3)->lexeme;
            symbolTables[currentProcedure][node->children.at(1)->children.at(1)->lexeme]
                .initialValue = node->children.at(3)->lexeme;
        }
    } else if (node->symbol == "procedure") {
        if (node->children.size() == 12) {
            string procedureName = node->children.at(1)->lexeme;
            if (procedureSignatures.find(procedureName) != procedureSignatures.end()) {
                errorMessages.push_back("ERROR: Duplicate procedure " + procedureName);
            }
            // Assume all procedures return int for now
            // cout << "procedureName: " << procedureName << endl;

            // procedureSignatures[procedureName] = signature;
            procedureSignatures[procedureName].returnType = "int";
            annotateTypes(node->children.at(3), procedureName);
            annotateTypes(node->children.at(6), procedureName);
            annotateTypes(node->children.at(7), procedureName);
            annotateTypes(node->children.at(9), procedureName);
            procedureSignatures[procedureName].returnExpression = node->children.at(9);
            procedureSignatures[procedureName].statements = node->children.at(7);
        }
    } else if (node->symbol == "main") {
        if (node->children.size() == 14) {
            annotateTypes(node->children.at(3), "wain");
            annotateTypes(node->children.at(5), "wain");
            annotateTypes(node->children.at(8), "wain");
            annotateTypes(node->children.at(9), "wain");
            annotateTypes(node->children.at(11), "wain");
            procedureSignatures["wain"].returnType = node->children.at(11)->type;
            procedureSignatures["wain"].parameterTypes.push_back(
                node->children.at(3)->children.at(1)->type);
            procedureSignatures["wain"].parameterTypes.push_back(
                node->children.at(5)->children.at(1)->type);
            symbolTables["wain"][node->children.at(3)->children.at(1)->lexeme].variableType =
                "parameter";
            symbolTables["wain"][node->children.at(5)->children.at(1)->lexeme].variableType =
                "parameter";
            symbolTables["wain"][node->children.at(3)->children.at(1)->lexeme].initialValue =
                node->children.at(3)->children.at(1)->type == "int" ? "0" : "NULL";
            symbolTables["wain"][node->children.at(5)->children.at(1)->lexeme].initialValue =
                node->children.at(5)->children.at(1)->type == "int" ? "0" : "NULL";
            procedureSignatures["wain"].parameterNames.push_back(
                node->children.at(3)->children.at(1)->lexeme);
            procedureSignatures["wain"].parameterNames.push_back(
                node->children.at(5)->children.at(1)->lexeme);
            procedureSignatures["wain"].returnExpression = node->children.at(11);
            procedureSignatures["wain"].statements = node->children.at(9);
        }
    }

    else if (node->symbol == "paramlist") {
        if (node->children.size() == 1) {
            annotateTypes(node->children.at(0), currentProcedure);
            string paramName = node->children.at(0)->children.at(1)->lexeme;
            procedureSignatures[currentProcedure].parameterTypes.push_back(
                node->children.at(0)->children.at(1)->type);
            procedureSignatures[currentProcedure].parameterNames.push_back(paramName);
            symbolTables[currentProcedure][paramName].variableType = "parameter";

        } else if (node->children.size() == 3) {
            annotateTypes(node->children.at(0), currentProcedure);
            annotateTypes(node->children.at(2), currentProcedure);
            string paramName = node->children.at(0)->children.at(1)->lexeme;
            procedureSignatures[currentProcedure].parameterTypes.push_back(
                node->children.at(0)->children.at(1)->type);
            procedureSignatures[currentProcedure].parameterNames.push_back(paramName);
            symbolTables[currentProcedure][paramName].variableType = "parameter";
        }
    } else if (node->symbol == "statement") {
        if (node->children.size() == 4 && node->children[1]->symbol == "BECOMES") {
            annotateTypes(node->children.at(0), currentProcedure);  // lvalue
            annotateTypes(node->children[2], currentProcedure);     // expr
            if (node->children.at(0)->type != node->children[2]->type) {
                errorMessages.push_back("ERROR: Type mismatch in statement");
            }

            string lvalueVar = getVariableFromLvalue(node->children.at(0));
            // cerr << "lvalueVar: " << lvalueVar << endl;
            if (!lvalueVar.empty()) {
                symbolTables[currentProcedure][lvalueVar].value = "NULL";
            }
        } else if (node->children.size() == 11) {
            annotateTypes(node->children.at(2), currentProcedure);
            annotateTypes(node->children.at(5), currentProcedure);
            annotateTypes(node->children.at(9), currentProcedure);

        } else if (node->children.size() == 7) {
            annotateTypes(node->children.at(2), currentProcedure);
            annotateTypes(node->children.at(5), currentProcedure);
        } else if (node->children.size() == 5) {
            if (node->children.at(0)->symbol == "PRINTLN" ||
                node->children.at(0)->symbol == "PUTCHAR") {
                annotateTypes(node->children.at(2), currentProcedure);
                if (node->children.at(2)->type != "int") {
                    errorMessages.push_back("ERROR: Expected int in" +
                                            node->children.at(0)->symbol);
                }
            } else if (node->children.at(0)->symbol == "DELETE") {
                annotateTypes(node->children.at(3), currentProcedure);
                if (node->children.at(3)->type != "int*") {
                    errorMessages.push_back("ERROR: Expected int* in DELETE");
                }
            }
        }
    } else if (node->symbol == "test") {
        if (node->children.size() == 3) {
            annotateTypes(node->children.at(0), currentProcedure);
            annotateTypes(node->children.at(2), currentProcedure);
            if (node->children.at(0)->type != node->children.at(2)->type) {
                errorMessages.push_back("ERROR: Type mismatch in test");
            }
        }
    }

    else {
        for (auto& child : node->children) {
            annotateTypes(child, currentProcedure);
        }
    }
}
void processArgList(const shared_ptr<ParseNode>& node, vector<string>& argTypes,
                    const string& currentProcedure) {
    if (node->symbol == "arglist") {
        if (node->children.size() == 1) {  // arglist → expr
            auto exprNode = node->children.at(0);
            annotateTypes(exprNode, currentProcedure);
            argTypes.insert(argTypes.begin(), exprNode->type);
        } else if (node->children.size() == 3) {  // arglist → expr COMMA arglist
            auto exprNode = node->children.at(0);
            annotateTypes(exprNode, currentProcedure);
            argTypes.insert(argTypes.begin(), exprNode->type);

            // Recursive call for the rest of the arglist
            processArgList(node->children[2], argTypes, currentProcedure);
        }
    }
}
string getVariableFromLvalue(const shared_ptr<ParseNode>& lvalueNode) {
    if (lvalueNode->children.size() == 1 && lvalueNode->children.at(0)->symbol == "ID") {
        return lvalueNode->children.at(0)->lexeme;
    } else if (lvalueNode->children.size() == 3 && lvalueNode->children.at(0)->symbol == "LPAREN" &&
               lvalueNode->children[2]->symbol == "RPAREN") {
        return getVariableFromLvalue(lvalueNode->children[1]);
    } else if (lvalueNode->children.size() == 2) {
        if (lvalueNode->children.at(0)->symbol == "STAR") {
            return getVariableFromLvalue(lvalueNode->children[1]);
        }
    } else if (lvalueNode->symbol == "factor") {
        if (lvalueNode->children.size() == 1 && lvalueNode->children.at(0)->symbol == "ID") {
            return lvalueNode->children.at(0)->lexeme;
        }
    }
    return "";
}
bool isNumber(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}
void generateBinaryOperation(const shared_ptr<ParseNode>& node, string& g,
                             const string& currentProcedure);
void generateArgList(const shared_ptr<ParseNode>& node, string& g, const string& currentProcedure) {
    if (node->symbol == "arglist") {
        if (node->children.size() == 1) {
            generateBinaryOperation(node->children.at(0), g, currentProcedure);
            g += "sw $3, -4($30)\n";
            g += "sub $30, $30, $4\n";
        } else if (node->children.size() == 3) {
            generateArgList(node->children[2], g, currentProcedure);
            generateBinaryOperation(node->children.at(0), g, currentProcedure);
            g += "sw $3, -4($30)\n";
            g += "sub $30, $30, $4\n";
        }
    }
}
void save29And31(string& g) {
    g += "; Save $29 and $31\n";
    g += "sw $29, -4($30)\n";
    g += "sub $30, $30, $4\n";
    g += "sw $31, -4($30)\n";
    g += "sub $30, $30, $4\n";
}
void restore29And31(string& g) {
    g += "; Restore $29 and $31\n";
    g += "lw $31, 0($30)\n";
    g += "add $30, $30, $4\n";
    g += "lw $29, 0($30)\n";
    g += "add $30, $30, $4\n";
}
int countArgs(const shared_ptr<ParseNode>& node) {
    if (node->symbol == "arglist") {
        if (node->children.size() == 1) {
            return 1;
        } else if (node->children.size() == 3) {
            return 1 + countArgs(node->children[2]);
        }
    }
    return 0;
}
void generateBinaryOperation(const shared_ptr<ParseNode>& node, string& g,
                             const string& currentProcedure) {
    if (node->children.size() == 0) {
        return;
    }
    if (node->symbol == "expr" || node->symbol == "term") {
        if (node->children.size() == 3) {
            generateBinaryOperation(node->children.at(0), g, currentProcedure);
            g += "sw $3, -4($30) ; push left operand\n";
            g += "sub $30, $30, $4\n";

            generateBinaryOperation(node->children[2], g, currentProcedure);
            g += "lw $5, 0($30) ; pop left operand into $5\n";
            g += "add $30, $30, $4\n";

            string op = node->children[1]->lexeme;
            if (op == "+") {
                if (node->children.at(0)->type == "int*" && node->children[2]->type == "int") {
                    g += "mult $3, $4\n";
                    g += "mflo $3\n";
                    g += "add $3, $5, $3\n";
                } else if (node->children.at(0)->type == "int" &&
                           node->children[2]->type == "int*") {
                    g += "mult $5, $4\n";
                    g += "mflo $5\n";
                    g += "add $3, $5, $3\n";
                } else {
                    g += "add $3, $5, $3\n";
                }
            } else if (op == "-") {
                if (node->children.at(0)->type == "int*" && node->children[2]->type == "int") {
                    g += "mult $3, $4\n";
                    g += "mflo $3\n";
                    g += "sub $3, $5, $3\n";
                } else if (node->children.at(0)->type == "int*" &&
                           node->children[2]->type == "int*") {
                    g += "sub $3, $5, $3\n";
                    g += "div $3, $4\n";
                    g += "mflo $3\n";
                } else {
                    g += "sub $3, $5, $3\n";
                }
            } else if (op == "*") {
                g += "mult $5, $3\n";
                g += "mflo $3\n";
            } else if (op == "/") {
                g += "div $5, $3\n";
                g += "mflo $3\n";
            } else if (op == "%") {
                g += "div $5, $3\n";
                g += "mfhi $3\n";
            }
        } else if (node->children.size() == 1) {
            generateBinaryOperation(node->children.at(0), g, currentProcedure);
        }
    } else if (node->symbol == "factor") {
        // printTreeHelper(node);
        if (node->children.empty()) {
            g += "lis $3\n";
            g += ".word 1\n";
            return;
        }

        if (node->children.at(0)->symbol == "NUM") {
            g += "lis $3\n";
            g += ".word " + node->children.at(0)->lexeme + "\n";
        } else if (node->children.at(0)->symbol == "ID") {
            if (node->children.size() == 1) {
                string varName = node->children.at(0)->lexeme;
                int offset = symbolTables[currentProcedure][varName].offset;
                g += "lw $3, " + to_string(offset) + "($29) ; load " + varName + "\n";
            } else if (node->children.size() >= 3) {
                // Function call
                string funcName = node->children.at(0)->lexeme;

                save29And31(g);

                // Push arguments
                if (node->children.size() == 4) {
                    generateArgList(node->children[2], g, currentProcedure);
                }

                // Call function
                g += "lis $5\n";
                if (funcName == "init" || funcName == "new" || funcName == "delete" ||
                    funcName == "print") {
                    g += ".word " + funcName + "\n";
                } else {
                    g += ".word procedure" + funcName + "\n";
                }
                g += "jalr $5\n";

                // Pop arguments
                if (node->children.size() == 4) {
                    int argCount = countArgs(node->children[2]);
                    g += "lis $5\n";
                    g += ".word " + to_string(4 * argCount) + "\n";
                    g += "add $30, $30, $5\n";
                }

                restore29And31(g);
            }
        } else if (node->children.at(0)->symbol == "LPAREN") {
            generateBinaryOperation(node->children[1], g, currentProcedure);
        } else if (node->children.at(0)->symbol == "GETCHAR") {
            g += "lis $5\n";
            g += ".word 0xffff0004\n";
            g += "lw $3, 0($5)\n";
        } else if (node->children.at(0)->symbol == "NULL") {
            g += "lis $3\n";
            g += ".word 1\n";
        } else if (node->children.at(0)->symbol == "AMP") {
            generateBinaryOperation(node->children[1], g, currentProcedure);
            string varName = getVariableFromLvalue(node->children[1]);

            int offset = symbolTables[currentProcedure][varName].offset;
            if (!varName.empty()) {
                g += "lis $3\n";
                g += ".word " + to_string(offset) + "\n";
                g += "add $3, $29, $3; address of " + varName + "\n";
            }

        } else if (node->children.at(0)->symbol == "STAR") {
            if (node->children.at(1)->symbol == "NULL") {
                g += "lis $3\n";
                g += ".word 1\n";
            } else {
                generateBinaryOperation(node->children.at(1), g, currentProcedure);
                g += "lw $3, 0($3)\n";
            }
        } else if (node->children.at(0)->symbol == "NEW") {
            cerr << "NEW" << endl;
            generateBinaryOperation(node->children[3], g, currentProcedure);
            g += "add $1, $3, $0 ; move array size to $1\n";
            // g += "lis $5\n";
            // g += ".word 1\n";
            // g += "add $1, $1, $5 ; add 1 to size for metadata\n";
            g += "lis $5\n";
            g += ".word new\n";
            save29And31(g);
            g += "jalr $5\n";
            restore29And31(g);
            g += "bne $3, $0, 1 ; skip next instruction if allocation succeeded\n";
            g += "add $3, $0, $11 ; set $3 to 1 (NULL) if allocation failed\n";
            // g += "beq $3, $0, 2 ; skip next two instructions if allocation failed\n";
            // g += "sw $1, -4($3) ; store size in metadata\n";
            // g += "lis $5\n";
            // g += ".word 4\n";
            // g += "add $3, $3, $5 ; move $3 past metadata\n";
        }
    } else if (node->symbol == "lvalue") {
        if (node->children.at(0)->symbol == "STAR") {
            if (node->children.at(1)->children.at(0)->symbol == "ID") {
                generateBinaryOperation(node->children.at(1), g, currentProcedure);
                g += "lw $3, 0($3)\n";
            } else if (node->children.at(1)->children.at(0)->symbol == "LPAREN") {
                generateBinaryOperation(node->children.at(1), g, currentProcedure);
            }
        }
    }
}
void generateTest(const shared_ptr<ParseNode>& node, string& g, const string& falseLabel,
                  const string& currentProcedure) {
    generateBinaryOperation(node->children.at(0), g, currentProcedure);
    g += "sw $3, -4($30) ; push left operand\n";
    g += "sub $30, $30, $4\n";

    generateBinaryOperation(node->children[2], g, currentProcedure);
    g += "lw $5, 0($30) ; pop left operand into $5\n";
    g += "add $30, $30, $4\n";

    string op = node->children[1]->lexeme;
    bool isPointerComparison =
        node->children.at(0)->type == "int*" || node->children[2]->type == "int*";

    if (isPointerComparison) {
        // For pointer comparisons, use sltu (Set on Less Than Unsigned)
        if (op == ">=") {
            g += "sltu $6, $5, $3\n";
            g += "bne $6, $0, " + falseLabel + "\n";
        } else if (op == "<") {
            g += "sltu $6, $5, $3\n";
            g += "beq $6, $0, " + falseLabel + "\n";
        } else if (op == "<=") {
            g += "sltu $6, $3, $5\n";
            g += "bne $6, $0, " + falseLabel + "\n";
        } else if (op == ">") {
            g += "sltu $6, $3, $5\n";
            g += "beq $6, $0, " + falseLabel + "\n";
        } else if (op == "==") {
            g += "bne $5, $3, " + falseLabel + "\n";
        } else if (op == "!=") {
            g += "beq $5, $3, " + falseLabel + "\n";
        }
    } else {
        // For integer comparisons, use slt (Set on Less Than)
        if (op == "==") {
            g += "bne $5, $3, " + falseLabel + "\n";
        } else if (op == "!=") {
            g += "beq $5, $3, " + falseLabel + "\n";
        } else if (op == "<") {
            g += "slt $6, $5, $3\n";
            g += "beq $6, $0, " + falseLabel + "\n";
        } else if (op == "<=") {
            g += "slt $6, $3, $5\n";
            g += "bne $6, $0, " + falseLabel + "\n";
        } else if (op == ">") {
            g += "slt $6, $3, $5\n";
            g += "beq $6, $0, " + falseLabel + "\n";
        } else if (op == ">=") {
            g += "slt $6, $5, $3\n";
            g += "bne $6, $0, " + falseLabel + "\n";
        }
    }
}
void generateStatements(const shared_ptr<ParseNode>& node, string& g,
                        const string& currentProcedure) {
    // for (const auto& entry : symbolTables[currentProcedure]) {
    //     cerr << "Variable Name: " << entry.first << endl;
    // }.
    // cerr << "symbol: " << node->symbol << endl;
    // cerr << "children size: " << node->children.size() << endl;
    if (node->children.size() == 0) {
        return;
    }
    if (node->symbol == "statements") {
        for (const auto& child : node->children) {
            generateStatements(child, g, currentProcedure);
        }
    } else if (node->symbol == "statement") {
        if (node->children.at(0)->symbol == "IF") {
            string elseLabel = generateLabel();
            string endLabel = generateLabel();

            generateTest(node->children[2], g, elseLabel, currentProcedure);
            generateStatements(node->children[5], g, currentProcedure);
            g += "beq $0, $0, " + endLabel + "\n";
            g += elseLabel + ":\n";
            generateStatements(node->children[9], g, currentProcedure);
            g += endLabel + ":\n";
        } else if (node->children.at(0)->symbol == "WHILE") {
            string startLabel = generateLabel();
            string endLabel = generateLabel();

            g += startLabel + ":\n";
            generateTest(node->children[2], g, endLabel, currentProcedure);
            generateStatements(node->children[5], g, currentProcedure);
            g += "beq $0, $0, " + startLabel + "\n";
            g += endLabel + ":\n";
        } else if (node->children[1]->symbol == "BECOMES") {
            if (node->children.at(0)->children.at(0)->symbol == "STAR") {
                generateBinaryOperation(node->children.at(0)->children[1], g, currentProcedure);
                g += "sw $3, -4($30) ; push address\n";
                g += "sub $30, $30, $4\n";
                generateBinaryOperation(node->children[2], g, currentProcedure);
                g += "lw $5, 0($30) ; pop address\n";
                g += "add $30, $30, $4\n";
                g += "sw $3, 0($5) ; store to dereferenced address\n";
            } else {
                string lvalue = getVariableFromLvalue(node->children.at(0));
                // cerr << "lvalue: " << lvalue << endl;
                // printTreeHelper(node->children[2]);
                generateBinaryOperation(node->children[2], g, currentProcedure);
                g += "sw $3, " + to_string(symbolTables[currentProcedure][lvalue].offset) +
                     "($29) ; store to " + lvalue + "\n";
            }
        } else if (node->children.at(0)->symbol == "PUTCHAR") {
            generateBinaryOperation(node->children[2], g, currentProcedure);
            g += "lis $5\n";
            g += ".word 0xffff000c\n";
            g += "sw $3, 0($5)\n";
        } else if (node->children.at(0)->symbol == "PRINTLN") {
            generateBinaryOperation(node->children[2], g, currentProcedure);
            g += "add $1, $3, $0 ; move value to $1 for println\n";
            g += "lis $5\n";
            g += ".word print\n";

            save29And31(g);
            g += "jalr $5\n";
            restore29And31(g);
        } else if (node->children.at(0)->symbol == "DELETE") {
            generateBinaryOperation(node->children[3], g, currentProcedure);

            g += "beq $3, $11, 11 ; skip deletion if NULL\n";
            // g += "lis $5\n";
            // g += ".word 4\n";
            // g += "sub $3, $3, $5 ; move back to metadata\n";
            g += "add $1, $3, $0 ; move address to $1 for delete\n";
            g += "lis $5\n";
            g += ".word delete\n";
            save29And31(g);
            g += "jalr $5\n";
            restore29And31(g);
        }
    }
}
void generateProcedure(const string& procName, string& g) {
    g += "procedure" + procName + ":\n";

    // Prologue
    g += "; begin prologue\n";
    g += "sub $29, $30, $4 ; set up frame pointer\n";

    const auto& signature = procedureSignatures[procName];
    int paramCount = signature.parameterNames.size();

    // Set up parameter offsets
    int j = 1;
    for (int i = paramCount - 1; i >= 0; --i) {
        cerr << "param: " << signature.parameterNames[i] << endl;
        symbolTables[procName][signature.parameterNames[i]].offset = 4 * j;
        j++;
    }

    // Allocate space for local variables
    int currentOffset = 0;
    for (auto& [variableName, tableEntry] : symbolTables[procName]) {
        if (tableEntry.variableType == "variable" &&
            (tableEntry.value != "" || tableEntry.initialValue != "")) {
            if (isNumber(tableEntry.initialValue)) {
                g += "lis $3\n";
                g += ".word " + tableEntry.initialValue + "\n";
                g += "sw $3, -4($30) ; push variable " + variableName + "\n";
                g += "sub $30, $30, $4 ; update stack pointer\n";
                tableEntry.offset = currentOffset;
                currentOffset -= 4;
            } else if (tableEntry.initialValue == "NULL") {
                g += "lis $3\n";
                g += ".word 1\n";
                g += "sw $3, -4($30) ; push NULL VALUE 1 to variable " + variableName + "\n";
                g += "sub $30, $30, $4 ; update stack pointer\n";
                tableEntry.offset = currentOffset;
                currentOffset -= 4;
            }
        }
    }

    g += "; end prologue\n\n";

    // Generate code for statements
    generateStatements(signature.statements, g, procName);

    // Generate code for return expression
    generateBinaryOperation(signature.returnExpression, g, procName);

    // Epilogue
    g += "\n; begin epilogue\n";
    g += "add $30, $29, $4 ; restore stack pointer\n";
    g += "jr $31 ; return to caller\n";
    g += "; end epilogue\n\n";
}
void generateWain(string& g) {
    g += ".import print\n";
    g += ".import init\n";
    g += ".import new\n";
    g += ".import delete\n";
    g += "; begin prologue\n";
    g += "lis $4\n";
    g += ".word 4\n";
    g += "lis $11\n";
    g += ".word 1 ; set $11 to 1 (NULL)\n";
    g += "sw $1, -4($30) ; push parameter variable " +
         procedureSignatures["wain"].parameterNames.at(0) + "\n";
    g += "sub $30, $30, $4 ; update stack pointer\n";
    symbolTables["wain"][procedureSignatures["wain"].parameterNames.at(0)].offset = 4;
    g += "sw $2, -4($30) ; push parameter variable " +
         procedureSignatures["wain"].parameterNames.at(1) + "\n";
    g += "sub $30, $30, $4 ; update stack pointer\n";
    symbolTables["wain"][procedureSignatures["wain"].parameterNames.at(1)].offset = 4;
    symbolTables["wain"][procedureSignatures["wain"].parameterNames.at(0)].offset += 4;
    g += "sub $29, $30, $4 ;  set frame pointer ( after pushing parameters , before pushing "
         "non-parameters \n";
    int currentOffset = 0;
    for (auto& [variableName, tableEntry] : symbolTables["wain"]) {
        if (tableEntry.variableType == "variable" &&
            (tableEntry.value != "" || tableEntry.initialValue != "")) {
            if (isNumber(tableEntry.initialValue)) {
                g += "lis $3\n";
                g += ".word " + tableEntry.initialValue + "\n";
                g += "sw $3, -4($30) ; push variable " + variableName + "\n";
                g += "sub $30, $30, $4 ; update stack pointer\n";
                tableEntry.offset = currentOffset;
                currentOffset -= 4;
            } else if (tableEntry.initialValue == "NULL") {
                g += "lis $3\n";
                g += ".word 1\n";
                g += "sw $3, -4($30) ; push NULL VALUE 1 to variable " + variableName + "\n";
                g += "sub $30, $30, $4 ; update stack pointer\n";
                tableEntry.offset = currentOffset;
                currentOffset -= 4;
            }
        }
    }

    if (procedureSignatures["wain"].parameterTypes.at(0) == "int") {
        g += "add $2, $0, $0 ; set $2 to 0 for init\n";
    } else {
        g += "add $2, $2, $0 ; use second parameter for init\n";
    }
    save29And31(g);

    g += "lis $5\n";
    g += ".word init\n";
    g += "jalr $5\n";

    restore29And31(g);

    g += "; end prologue\n";

    // for (const auto& [lvalue, rvalue] : assignmentOrder) {
    //     if (isNumber(rvalue)) {
    //         g += "lis $3\n";
    //         g += ".word " + rvalue + "\n";
    //     } else {
    //         g += "lw $3, " + to_string(symbolTables["wain"][rvalue].offset) + "($29) ; load " +
    //              rvalue + "\n";
    //     }
    //     g += "sw $3, " + to_string(symbolTables["wain"][lvalue].offset) + "($29) ; store to " +
    //          lvalue + "\n";
    // }
    generateStatements(procedureSignatures["wain"].statements, g, "wain");
    auto returnExpr = procedureSignatures["wain"].returnExpression;
    printTreeHelper(returnExpr);
    generateBinaryOperation(returnExpr, g, "wain");

    g += "; begin epilogue\n";
    for (auto& [variableName, tableEntry] : symbolTables["wain"]) {
        g += "add $30, $30, $4\n";
    }
    g += "jr $31\n";
}
void generateCode(string& g) {
    // g += ".import print\n";
    generateWain(g);
    for (const auto& [procName, signature] : procedureSignatures) {
        if (procName != "wain") {
            generateProcedure(procName, g);
        }
    }
}

int main() {
    vector<string> input;
    string line;
    while (getline(cin, line)) {
        input.push_back(line);
    }

    buildTree(input);
    std::unordered_map<std::string, std::vector<shared_ptr<ParseNode>>> procedureParameters;
    std::unordered_map<std::string, shared_ptr<ParseNode>> procedureReturnExpressions;
    annotateTypes(root, "wain");
    cerr << "Annotated types" << endl;

    // cout << "Number of procedures: " << procedureSignatures.size() << endl;

    string generatedCode;
    generateCode(generatedCode);

    // printProcedureSignatures(procedureSignatures);
    // printSymbolTables();

    cout << generatedCode << endl;
    // printTree();

    return 0;
}