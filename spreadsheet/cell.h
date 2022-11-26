// Яндекс-Практикум 2022. Дипломный проект по профессии "Разработчик С++". Черепухин Евгений Сергеевич 16 когорта.
#pragma once
#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

class Sheet;


class Cell : public CellInterface {
public:
    using Positions = std::unordered_set<Position, HashPosition>;

    Cell(Sheet& sheet, Position pos);

    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells()const override;
    bool IsReferenced() const;
    bool IsEmpty() const;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl {
    public:
        virtual ~Impl() = default;

        virtual Value GetValue() const = 0;

        virtual std::string GetText() const = 0;

        virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl();

        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string str);

        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string str_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string str,
            const SheetInterface& sheet);

        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

    private:
        const SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
    };

    Sheet& sheet_;
    Position pos_;
    std::unique_ptr<Impl> impl_;
    Positions dependents_;
    Positions effects_;
    mutable std::optional<Value> cache_;

    std::unique_ptr<Impl> MakeImpl(std::string text) const;
    Cell* MakeCell(Position pos) const;
    bool IsCyclic(const Impl* impl) const;
    bool IsCyclicFormula(const Positions& dependents, Positions& checkeds) const;
    void UpdateCache() const;
    void InvalidCache();
    void RemoveOldDependents();
    void CreateNewDependents();
    void InvalidAllDependentCaches(const Positions& effects, Positions& invalids);
    void Init();    
};