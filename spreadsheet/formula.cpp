#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <utility>


using namespace std::literals;

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(std::move(expression))) {}
    
    Value Evaluate(const SheetInterface& sheet) const override{        
        CellLookup cell_lookup = [&sheet](Position pos) {
            auto cell = sheet.GetCell(pos);
            if (cell == nullptr) {
                return 0.0;
            }
            auto value = cell->GetValue();
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
            else if (std::holds_alternative<std::string>(value)) {
                std::string text = sheet.GetCell(pos)->GetText();
                if (text.empty()) {
                    return 0.0;
                }
                
                if (text.front() == ESCAPE_SIGN) {
                    throw FormulaError(FormulaError::Category::Value);
                }
                try {
                    for (char c : text) {
                        if (!std::isdigit(c)) {
                            throw FormulaError(FormulaError::Category::Value);
                        }
                    }
                    return std::stod(text);
                }
                catch (...) {
                    throw FormulaError(FormulaError::Category::Value);
                }
            }
            else {
                throw std::get<FormulaError>(value);
            }
        };
        try {
            return ast_.Execute(cell_lookup);
        }
        catch(FormulaError &err){
            return err;
        }
    } 
        
    std::string GetExpression() const override {
            std::ostringstream os;
            ast_.PrintFormula(os);
            return os.str();
        }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> result;        
        // Убираем дублирование ячеек, для чего используем set
        std::set<Position> positions_(ast_.GetCells().begin(), ast_.GetCells().end());
        for (const auto& pos : positions_) {
            result.push_back(pos);
        }
        return result;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}