#pragma once

#include "cell.h"
#include "common.h"


#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <vector>

class Cell;


class Sheet : public SheetInterface {
public:
    using Cells = std::unordered_map<Position, std::unique_ptr<Cell>, HashPosition>;

    ~Sheet() = default;

    void SetCell(Position pos,  std::string text) ;

    const CellInterface* GetCell(Position pos) const override;

    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;

    void PrintTexts(std::ostream& output) const override;

private:
	// Можете дополнить ваш класс нужными полями и методами
    // Проверяет позицию ячейки. Если ячейка не валидна, выбрасывает исключение.
    void PositionCorrect(Position pos) const;

    // Создаёт ячейку в заданной позизии.
    void MakeCell(Position pos, std::string text);

    // Шаблонный метод для вывода таблицы.
    template<typename Print>
    void PrintSheet(std::ostream& output, Print print) const;

    Cells cells_;

    Positions positions_;
    
};

template <typename Print>
void Sheet::PrintSheet(std::ostream& output, Print print) const {
    Size scope = GetPrintableSize();
    for (int i = 0; i < scope.rows; ++i) {
        for (int j = 0; j < scope.cols - 1; ++j) {
            Position pos{ i, j };
            print((cells_.count(pos) != 0) ? cells_.at(pos).get() : nullptr);
            output << '\t';
        }
        Position pos{ i, scope.cols - 1 };
        print((cells_.count(pos) != 0) ? cells_.at(pos).get() : nullptr);
        output << '\n';
    }
}