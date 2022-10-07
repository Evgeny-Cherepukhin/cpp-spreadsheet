#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    PositionCorrect(pos);
    if (!text.empty()) {
        positions_.insert(pos);
    }
    if (cells_.count(pos) == 0) {
        MakeCell(pos, std::move(text));
    }
    else {
        cells_[pos]->Set(std::move(text));
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    PositionCorrect(pos);
    if (cells_.count(pos)== 0) {
        return nullptr;
    }
    Cell* cell = cells_.at(pos).get();
    if (!cell || (cell->IsEmpty() && !cell->IsReferenced())) {
        return nullptr;
    }
    return cell;
}

void Sheet::ClearCell(Position pos) {
    SetCell(pos, std::string());
    positions_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    int max_r = 0;
    int max_c = 0;
    for (Position pos : positions_) {
        max_r = std::max(max_r, pos.row + 1);
        max_c = std::max(max_c, pos.col + 1);
    }
    return { max_r, max_c };
}

void Sheet::PrintValues(std::ostream& output) const {
    auto get_value = [&output](const Cell* cell) {
        if (cell) {
            std::visit([&output](const auto& x) {
                output << x;
                }, cell->GetValue());
        }
    };
    PrintSheet(output, get_value);
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto get_text = [&output](const Cell* cell) {
        if (cell) {
            output << cell->GetText();
        }
    };
    PrintSheet(output, get_text);
}

void Sheet::PositionCorrect(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("The position is incorrect"s);
    }
}

void Sheet::MakeCell(Position pos, std::string text) {
    Cell* cell = new Cell(*this, pos);
    cell->Set(std::move(text));
    cells_[pos].reset(cell);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}