// Яндекс-Практикум 2022. Дипломный проект по профессии "Разработчик С++". Черепухин Евгений Сергеевич 16 когорта.
#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>
#include <utility>


using namespace std::literals;

// --------------------Cell::EmptyImpl---------------------

Cell::EmptyImpl::EmptyImpl()
    : Impl() {}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return std::string();
}

std::string Cell::EmptyImpl::GetText() const {
    return std::string();
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}

// --------------------Cell::TextImpl---------------------

Cell::TextImpl::TextImpl(std::string str)
    : Impl()
    , str_(std::move(str)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    return (str_.front() != ESCAPE_SIGN) ? str_ : str_.substr(1);
}

std::string Cell::TextImpl::GetText() const {
    return str_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return {};
}

// --------------------Cell::FormulaImpl---------------------

Cell::FormulaImpl::FormulaImpl(std::string str,
    const SheetInterface& sheet)
    : Impl()
    , sheet_(sheet)
    , formula_(std::move(ParseFormula(str))) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    auto res = formula_->Evaluate(sheet_);
    if (std::holds_alternative<double>(res)) {
        return std::get<double>(res);
    }
    return std::get<FormulaError>(res);
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet, Position pos)
    : CellInterface()
    , sheet_(sheet)
    , pos_(pos)
    , impl_(std::make_unique<EmptyImpl>()) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    using namespace std::literals;
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else {
        auto tmp = MakeImpl(std::move(text));
        if (IsCyclic(tmp.get())) {
            throw CircularDependencyException("This formula is cyclical"s);
        }
        else {
            impl_ = std::move(tmp);
        }
    }        
    Init();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    UpdateCache();
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !dependents_.empty() || !effects_.empty();
}

bool Cell::IsEmpty() const {
    return impl_->GetText().empty();
}

std::unique_ptr<Cell::Impl> Cell::MakeImpl(std::string text) const {
    using namespace std::literals;
    if (text.empty()) {
        return std::make_unique<EmptyImpl>();
    }
    if (text.front() == FORMULA_SIGN && text.size() > 1) {
        try {
            return std::make_unique<FormulaImpl>(std::move(text.substr(1)), sheet_);
        }
        catch (...) {
            throw FormulaException("Sytnax error"s);
        }
    }
    else {
        return std::make_unique<TextImpl>(std::move(text));
    }
}

Cell* Cell::MakeCell(Position pos) const {
    Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
    if (!cell) {
        sheet_.SetCell(pos, std::string());
               
    }

    return cell;
}

bool Cell::IsCyclic(const Impl * impl) const {
    auto positions = impl->GetReferencedCells();
    Positions dependents(positions.begin(), positions.end());
    Positions checkeds;
    return IsCyclicFormula(dependents, checkeds);
}

bool Cell::IsCyclicFormula(const Positions & dependents, Positions & checkeds) const {
    if (dependents.count(pos_) != 0) {
        return true;
    }
    for (Position pos : dependents) {
        if (!pos.IsValid() || checkeds.count(pos) != 0) {
            continue;
        }
        checkeds.insert(pos);
        Cell* cell = MakeCell(pos);
        if (cell == NULL) {
            return false;
        }
        
        if (IsCyclicFormula(cell->dependents_, checkeds)) {
            return true;
        }
    }
    return false;
}

void Cell::UpdateCache() const {
    if (!cache_.has_value()) {
        cache_ = impl_->GetValue();
    }
}

void Cell::InvalidCache() {
    cache_ = std::nullopt;
}

void Cell::RemoveOldDependents() {
    for (Position pos : dependents_) {
        if (!pos.IsValid()) {
            continue;
        }
        Cell* cell = MakeCell(pos);
        cell->effects_.erase(pos);
    }
    dependents_.clear();
}

void Cell::CreateNewDependents() {
    for (Position pos : GetReferencedCells()) {
        if (!pos.IsValid()) {
            continue;
        }
        dependents_.insert(pos);
        Cell* cell = MakeCell(pos);
        cell->effects_.insert(pos);
    }
}

void Cell::InvalidAllDependentCaches(const Positions & effects, Positions & invalids) {
    for (Position pos : effects) {
        if (!pos.IsValid()) {
            continue;
        }
        Cell* cell = MakeCell(pos);
        cell->InvalidCache();
        invalids.insert(pos);        
    }
}

void Cell::Init() {
    InvalidCache();
    RemoveOldDependents();
    CreateNewDependents();
    Positions invalids;
    InvalidAllDependentCaches(effects_, invalids);
}