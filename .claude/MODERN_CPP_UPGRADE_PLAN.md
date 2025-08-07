# Modern C++ Upgrade Plan: C++17/C++20/C++23/C++26 Modernization

## Current State Analysis

**CMake Configuration**: C++20 is set in `CMakeLists.txt` but the codebase primarily uses C++17/earlier patterns.

**Good Modern Practices Already Present:**
- ✅ `nullptr` usage throughout (not `NULL`)
- ✅ `auto` type deduction in many places
- ✅ Range-based for loops: `for (auto *elm : elements)`
- ✅ Smart pointers: `std::shared_ptr`, `std::make_shared`
- ✅ Lambda expressions with capture
- ✅ `enum class` for type-safe enums
- ✅ Modern exception handling with custom `Pandaception`

**Areas for Improvement:**
- Heavy reliance on Qt containers (`QVector`, `QHash`, `QMap`)
- Limited use of C++17/C++20 standard library features
- Manual memory management in some areas
- String concatenation using `+` operators
- Verbose template syntax
- Missing constexpr/consteval opportunities

## Phase 1: C++17 Modernization Opportunities

### 1.1 Structured Bindings

**Current Pattern:**
```cpp
// app/elementmapping.cpp
for (auto *elm : std::as_const(m_elements)) {
    for (auto *inputPort : elm->inputs()) {
        // Complex iteration logic
    }
}
```

**C++17 Modernized:**
```cpp
for (const auto& [elementId, element] : m_elementMap) {
    for (const auto& [portIndex, port] : element->getIndexedPorts()) {
        // Cleaner iteration with structured bindings
    }
}

// For QMap/QHash iteration:
for (const auto& [key, value] : qAsConst(m_varMap)) {
    // Direct key-value access instead of iterators
}
```

### 1.2 `std::optional` for Better Null Handling

**Current Pattern:**
```cpp
// app/elementfactory.cpp
static GraphicElement *buildElement(const ElementType type) {
    // Returns nullptr on failure - easy to forget null checks
    switch (type) {
        case ElementType::And: return new And();
        default: return nullptr; // Dangerous!
    }
}
```

**C++17 Modernized:**
```cpp
static std::optional<std::unique_ptr<GraphicElement>> buildElement(ElementType type) {
    switch (type) {
        case ElementType::And: 
            return std::make_unique<And>();
        case ElementType::Or:
            return std::make_unique<Or>();
        default: 
            return std::nullopt; // Explicit failure
    }
}

// Usage:
if (auto element = ElementFactory::buildElement(type)) {
    scene->addElement(std::move(*element));
} else {
    // Handle error explicitly
}
```

### 1.3 `std::variant` for Type-Safe Unions

**Current Pattern:**
```cpp
// Multiple similar classes with different data
class InputButton : public GraphicElementInput { /* ... */ };
class InputSwitch : public GraphicElementInput { /* ... */ };
class InputRotary : public GraphicElementInput { /* ... */ };
```

**C++17 Modernized:**
```cpp
// app/element/input_types.h
enum class InputType { Button, Switch, Rotary };

struct ButtonData { bool pressed = false; };
struct SwitchData { bool toggled = false; };
struct RotaryData { int position = 0; int maxPositions = 8; };

using InputVariant = std::variant<ButtonData, SwitchData, RotaryData>;

class UnifiedInput : public GraphicElementInput {
public:
    explicit UnifiedInput(InputVariant data) : m_data(std::move(data)) {}
    
    void updateLogic() override {
        std::visit([this](const auto& data) {
            using T = std::decay_t<decltype(data)>;
            if constexpr (std::is_same_v<T, ButtonData>) {
                setOutputValue(data.pressed);
            } else if constexpr (std::is_same_v<T, SwitchData>) {
                setOutputValue(data.toggled);
            } else if constexpr (std::is_same_v<T, RotaryData>) {
                setOutputValue(data.position > 0);
            }
        }, m_data);
    }
    
private:
    InputVariant m_data;
};
```

### 1.4 `if constexpr` for Template Specialization

**Current Pattern:**
```cpp
// app/elementfactory.cpp - Large switch statement
std::shared_ptr<LogicElement> ElementFactory::buildLogicElement(GraphicElement *elm) {
    switch (elm->elementType()) {
        case ElementType::And: return std::make_shared<LogicAnd>(elm->inputSize());
        case ElementType::Or:  return std::make_shared<LogicOr>(elm->inputSize());
        // ... 30+ cases
    }
}
```

**C++17 Modernized:**
```cpp
// app/elementfactory.h
template<ElementType Type>
constexpr auto createLogicElement(int inputSize) {
    if constexpr (Type == ElementType::And) {
        return std::make_shared<LogicAnd>(inputSize);
    } else if constexpr (Type == ElementType::Or) {
        return std::make_shared<LogicOr>(inputSize);
    } else if constexpr (Type == ElementType::Not) {
        return std::make_shared<LogicNot>(inputSize);
    } // ... compile-time dispatch
}

// Runtime wrapper:
std::shared_ptr<LogicElement> buildLogicElement(ElementType type, int inputSize) {
    return std::visit([inputSize](auto typeConstant) {
        return createLogicElement<typeConstant>(inputSize);
    }, toVariant(type));
}
```

### 1.5 `std::string_view` for String Operations

**Current Pattern:**
```cpp
// app/arduino/codegenerator.cpp
QString CodeGenerator::removeForbiddenChars(const QString &input) {
    return input.toLower().trimmed().replace(' ', '_').replace('-', '_');
}
```

**C++17 Modernized:**
```cpp
// For performance-critical string operations
std::string_view extractElementName(std::string_view fullName) {
    const auto pos = fullName.find_last_of('_');
    return (pos != std::string_view::npos) ? fullName.substr(pos + 1) : fullName;
}

// Qt integration when needed
QString cleanIdentifier(QStringView input) {
    return QString(input).toLower().replace(QRegularExpression("\\W"), "_");
}
```

## Phase 2: C++20 Modernization Opportunities

### 2.1 Concepts for Type Constraints

**Current Pattern:**
```cpp
// app/logicelement.h - No compile-time type checking
template<typename T>
void connectElements(T* source, T* target) {
    // Hope T has the right interface!
}
```

**C++20 Modernized:**
```cpp
// app/concepts.h
#include <concepts>

template<typename T>
concept LogicElement = requires(T element) {
    { element.updateLogic() } -> std::same_as<void>;
    { element.inputValue(0) } -> std::same_as<bool>;
    { element.outputValue(0) } -> std::same_as<bool>;
    { element.inputSize() } -> std::convertible_to<int>;
    { element.outputSize() } -> std::convertible_to<int>;
};

template<typename T>
concept GraphicElement = requires(T element) {
    { element.elementType() } -> std::same_as<ElementType>;
    { element.pos() } -> std::convertible_to<QPointF>;
    { element.inputs() } -> std::ranges::range;
    { element.outputs() } -> std::ranges::range;
};

// Type-safe functions
template<LogicElement T>
void connectLogicElements(T& source, T& target, int sourcePort, int targetPort) {
    target.connectInput(targetPort, &source, sourcePort);
}

template<GraphicElement T>
auto getElementBounds(const std::vector<T*>& elements) {
    return std::ranges::views::transform(elements, [](const T* elem) {
        return elem->boundingRect();
    });
}
```

### 2.2 Ranges for Cleaner Algorithms

**Current Pattern:**
```cpp
// app/simulation.cpp
QVector<GraphicElement*> sortedElements;
std::sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
    return a->priority() < b->priority();
});
```

**C++20 Modernized:**
```cpp
#include <ranges>

// app/simulation.cpp
auto sortElementsByPriority(std::ranges::range auto&& elements) {
    return elements 
         | std::views::filter([](const auto* elem) { return elem->isValid(); })
         | std::views::transform([](auto* elem) { return elem; })
         | std::ranges::to<std::vector>()
         | std::ranges::actions::sort([](const auto* a, const auto* b) {
               return a->priority() < b->priority();
           });
}

// Usage:
auto simulationOrder = sortElementsByPriority(scene->elements())
                     | std::views::take_while([](const auto* elem) {
                           return elem->elementType() != ElementType::IC;
                       });

for (auto* element : simulationOrder) {
    element->updateLogic();
}
```

### 2.3 `std::format` for String Formatting

**Current Pattern:**
```cpp
// app/bewaveddolphin.cpp
QString fileName = QString("%1_%2_%3.vcd")
    .arg(m_currentDir)
    .arg(m_currentTime)
    .arg(m_signalCount);
```

**C++20 Modernized:**
```cpp
#include <format>

// For performance-critical paths
std::string formatSignalName(std::string_view baseName, int index, int totalBits) {
    return std::format("{}[{}:{}]", baseName, index, totalBits - 1);
}

// Qt integration wrapper
QString formatElementLabel(const QString& type, int id, const QString& properties) {
    auto formatted = std::format("{}_{}_{}", 
                                type.toStdString(), 
                                id, 
                                properties.toStdString());
    return QString::fromStdString(formatted);
}
```

### 2.4 Coroutines for Async Operations

**Current Pattern:**
```cpp
// File loading with blocking operations
void loadPandaFile(const QString& fileName) {
    QFile file(fileName);
    // Blocking I/O
    QByteArray data = file.readAll();
    // Long parsing operation blocks UI
    parseCircuitData(data);
}
```

**C++20 Modernized:**
```cpp
#include <coroutine>

// app/async_loader.h
struct FileLoadTask {
    struct promise_type {
        FileLoadTask get_return_object() { 
            return FileLoadTask{std::coroutine_handle<promise_type>::from_promise(*this)}; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    
    std::coroutine_handle<promise_type> coro;
    
    FileLoadTask(std::coroutine_handle<promise_type> h) : coro(h) {}
    ~FileLoadTask() { if (coro) coro.destroy(); }
};

FileLoadTask loadPandaFileAsync(const QString& fileName) {
    QFile file(fileName);
    
    // Yield control while reading
    co_await std::suspend_always{};
    QByteArray data = file.readAll();
    
    // Yield control during parsing  
    co_await std::suspend_always{};
    parseCircuitData(data);
    
    // Resume main thread
    co_return;
}
```

### 2.5 `consteval` for Compile-Time Computations

**Current Pattern:**
```cpp
// app/enums.h - Runtime enum-to-string conversion
QString ElementFactory::typeToText(const ElementType type) {
    return QVariant::fromValue(type).toString();
}
```

**C++20 Modernized:**
```cpp
// app/element_metadata.h
#include <string_view>
#include <array>

consteval auto makeElementTypeMap() {
    return std::array{
        std::pair{ElementType::And, std::string_view{"And"}},
        std::pair{ElementType::Or, std::string_view{"Or"}},
        std::pair{ElementType::Not, std::string_view{"Not"}},
        // ... all types
    };
}

constexpr auto ELEMENT_TYPE_MAP = makeElementTypeMap();

consteval std::string_view typeToStringView(ElementType type) {
    for (const auto& [enumVal, name] : ELEMENT_TYPE_MAP) {
        if (enumVal == type) return name;
    }
    return "Unknown";
}

// Runtime wrapper for Qt integration
QString typeToQString(ElementType type) {
    return QString::fromUtf8(typeToStringView(type));
}
```

## Phase 3: C++23 Modernization Opportunities

### 3.1 `std::expected` for Better Error Handling

**Current Pattern:**
```cpp
// app/bewaveddolphin.cpp
void BewavedDolphin::save(const QString &fileName) {
    // Throws exceptions for various error conditions
    if (fileName.isEmpty()) {
        throw PANDACEPTION("Missing file name.");
    }
    
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Could not open file: %1", file.errorString());
    }
    // ... more potential failures
}
```

**C++23 Modernized:**
```cpp
#include <expected>

// app/file_operations.h
enum class FileError {
    EmptyFileName,
    CannotOpen,
    WriteFailure,
    InvalidFormat,
    PermissionDenied
};

std::expected<void, FileError> saveWaveform(const QString& fileName, const WaveformData& data) {
    if (fileName.isEmpty()) {
        return std::unexpected(FileError::EmptyFileName);
    }
    
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return std::unexpected(FileError::CannotOpen);
    }
    
    if (auto result = writeWaveformData(file, data); !result) {
        return std::unexpected(result.error());
    }
    
    return {}; // Success
}

// Usage:
if (auto result = saveWaveform(fileName, data)) {
    // Success
    showStatusMessage("File saved successfully");
} else {
    // Handle specific errors
    switch (result.error()) {
        case FileError::EmptyFileName:
            showError("Please provide a file name");
            break;
        case FileError::CannotOpen:
            showError("Cannot open file - check permissions");
            break;
        // ... handle other cases
    }
}
```

### 3.2 `std::mdspan` for Multi-Dimensional Circuit Data

**Current Pattern:**
```cpp
// app/truth_table.cpp
// Truth table stored as flat array with manual indexing
class TruthTable {
    QBitArray m_key; // Flat storage, complex indexing
    
    bool getValue(int inputs, int outputs) {
        int index = calculateIndex(inputs, outputs); // Manual calculation
        return m_key.at(index);
    }
};
```

**C++23 Modernized:**
```cpp
#include <mdspan>

// app/truth_table.h
class ModernTruthTable {
public:
    using TruthData = std::vector<bool>;
    using TruthView = std::mdspan<bool, std::dextents<size_t, 2>>;
    
    ModernTruthTable(size_t inputCount, size_t outputCount) 
        : m_inputCount(inputCount)
        , m_outputCount(outputCount)
        , m_data(std::pow(2, inputCount) * outputCount)
        , m_view(m_data.data(), std::pow(2, inputCount), outputCount) {}
    
    bool getValue(size_t inputPattern, size_t outputIndex) const {
        return m_view[inputPattern, outputIndex]; // Clean 2D access
    }
    
    void setValue(size_t inputPattern, size_t outputIndex, bool value) {
        m_view[inputPattern, outputIndex] = value;
    }
    
    // Range-based iteration over rows/columns
    auto getInputRow(size_t inputPattern) {
        return std::ranges::subrange(
            m_view.data() + inputPattern * m_outputCount,
            m_view.data() + (inputPattern + 1) * m_outputCount
        );
    }
    
private:
    size_t m_inputCount, m_outputCount;
    TruthData m_data;
    TruthView m_view;
};
```

### 3.3 `std::print` and `std::println` for Modern Debugging

**Current Pattern:**
```cpp
// app/common.h - Custom debug macros
#define qCDebug(category) \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, category().categoryName()).debug().noquote().nospace()
```

**C++23 Modernized:**
```cpp
#include <print>

// app/debug.h - Type-safe debugging with std::print
template<typename... Args>
void debug_print(std::format_string<Args...> fmt, Args&&... args) {
    if constexpr (DEBUG_BUILD) {
        std::print(std::cerr, "[DEBUG] ");
        std::println(std::cerr, fmt, std::forward<Args>(args)...);
    }
}

template<typename... Args>  
void simulation_trace(std::format_string<Args...> fmt, Args&&... args) {
    if constexpr (TRACE_SIMULATION) {
        std::print(std::cerr, "[SIM] ");
        std::println(std::cerr, fmt, std::forward<Args>(args)...);
    }
}

// Usage:
void Simulation::update() {
    simulation_trace("Updating simulation at step {}", m_currentStep);
    
    for (auto* element : m_sortedElements) {
        debug_print("Processing element {} of type {}", 
                   element->id(), 
                   magic_enum::enum_name(element->type()));
        element->updateLogic();
    }
}
```

### 3.4 `std::flat_map` and `std::flat_set` for Performance

**Current Pattern:**
```cpp
// app/elementfactory.h
QMap<int, ItemWithId *> m_map; // Tree-based map
QMap<ElementType, ElementStaticProperties> m_propertyCache;
```

**C++23 Modernized:**
```cpp
#include <flat_map>

// app/elementfactory.h - Better cache performance
class ElementFactory {
private:
    // Contiguous storage for better cache performance
    std::flat_map<int, ItemWithId*> m_itemMap;
    std::flat_map<ElementType, ElementStaticProperties> m_propertyCache;
    
    // For small, frequently accessed collections
    std::flat_set<ElementType> m_supportedTypes;
    
public:
    // Batch insertion for optimal performance
    void registerItems(std::span<const std::pair<int, ItemWithId*>> items) {
        m_itemMap.insert_range(items);
    }
    
    // Range-based queries
    auto getItemsInRange(int minId, int maxId) const {
        return m_itemMap | std::views::filter([=](const auto& pair) {
            return pair.first >= minId && pair.first <= maxId;
        }) | std::views::values;
    }
};
```

### 3.5 `std::generator` for Lazy Evaluation

**Current Pattern:**
```cpp
// app/simulation.cpp - Eager evaluation
QVector<GraphicElement*> Simulation::getAllElements() {
    QVector<GraphicElement*> result;
    for (auto* element : m_scene->elements()) {
        if (element->isValid()) {
            result.append(element);
            // Add IC sub-elements
            if (element->elementType() == ElementType::IC) {
                auto* ic = qobject_cast<IC*>(element);
                result.append(ic->getInternalElements());
            }
        }
    }
    return result; // Large vector created upfront
}
```

**C++23 Modernized:**
```cpp
#include <generator>

// app/simulation.h - Lazy element traversal
std::generator<GraphicElement*> getAllElements() {
    for (auto* element : m_scene->elements()) {
        if (!element->isValid()) continue;
        
        co_yield element;
        
        // Lazy expansion of ICs
        if (element->elementType() == ElementType::IC) {
            auto* ic = static_cast<IC*>(element);
            for (auto* subElement : ic->getInternalElements()) {
                if (subElement->isValid()) {
                    co_yield subElement;
                }
            }
        }
    }
}

// Usage - elements generated on-demand
void Simulation::update() {
    for (auto* element : getAllElements()) {
        element->updateLogic(); // Process one at a time
    }
}
```

### 3.6 Deducing `this` for CRTP Simplification

**Current Pattern:**
```cpp
// app/graphicelement.h - Verbose CRTP pattern
template<typename Derived>
class GraphicElementBase : public QGraphicsObject {
public:
    void commonMethod() {
        static_cast<Derived*>(this)->specificMethod();
    }
};

class And : public GraphicElementBase<And> {
public:
    void specificMethod() { /* implementation */ }
};
```

**C++23 Modernized:**
```cpp
// app/graphicelement.h - Cleaner with deducing this
class GraphicElementBase : public QGraphicsObject {
public:
    template<typename Self>
    void commonMethod(this Self&& self) {
        self.specificMethod(); // No static_cast needed!
    }
    
    template<typename Self>
    auto getBounds(this const Self& self) {
        return self.boundingRect().united(self.childrenBoundingRect());
    }
};

class And : public GraphicElementBase {
public:
    void specificMethod() { /* implementation */ }
    QRectF boundingRect() const override { /* ... */ }
};
```

## Phase 4: C++26 Future-Proofing Opportunities

### 4.1 Pattern Matching with `inspect` (Proposed)

**Current Pattern:**
```cpp
// app/elementfactory.cpp - Large switch statements
std::shared_ptr<LogicElement> ElementFactory::buildLogicElement(GraphicElement *elm) {
    switch (elm->elementType()) {
        case ElementType::And: return std::make_shared<LogicAnd>(elm->inputSize());
        case ElementType::Or:  return std::make_shared<LogicOr>(elm->inputSize());
        case ElementType::Not: return std::make_shared<LogicNot>(elm->inputSize());
        // ... 30+ cases
        default: return nullptr;
    }
}
```

**C++26 Potential (if approved):**
```cpp
#include <experimental/pattern_matching>

// app/elementfactory.cpp - Pattern matching
std::shared_ptr<LogicElement> buildLogicElement(GraphicElement* elm) {
    return inspect(elm->elementType()) {
        ElementType::And => std::make_shared<LogicAnd>(elm->inputSize()),
        ElementType::Or  => std::make_shared<LogicOr>(elm->inputSize()),
        ElementType::Not => std::make_shared<LogicNot>(elm->inputSize()),
        
        // Pattern matching on multiple conditions
        [type] if (isSequentialElement(type)) => std::make_shared<SequentialLogic>(elm),
        [type] if (isCombinatorialElement(type)) => std::make_shared<CombinatorialLogic>(elm),
        
        // Pattern matching with guards
        ElementType::IC if (elm->hasInternalElements()) => expandIC(elm),
        ElementType::IC => std::make_shared<LogicNone>(),
        
        _ => nullptr // Default case
    };
}
```

### 4.2 Reflection and Metaprogramming Enhancement

**Current Pattern:**
```cpp
// app/enums.h - Manual enum string conversion
QString ElementFactory::typeToText(const ElementType type) {
    return QVariant::fromValue(type).toString();
}
```

**C++26 Potential:**
```cpp
#include <experimental/reflect>

// app/element_reflection.h - Compile-time reflection
template<typename EnumType>
constexpr std::string_view enumToString(EnumType value) {
    constexpr auto enumInfo = std::reflect(EnumType{});
    
    template for (constexpr auto enumerator : enumInfo.enumerators()) {
        if (enumerator.value() == value) {
            return enumerator.name();
        }
    }
    return "Unknown";
}

// Automatic serialization using reflection
template<typename T>
void serialize(const T& obj, QDataStream& stream) {
    constexpr auto typeInfo = std::reflect(T{});
    
    template for (constexpr auto member : typeInfo.data_members()) {
        if constexpr (member.is_public()) {
            stream << obj.[:member:]; // Splice operator
        }
    }
}

// Usage:
void GraphicElement::save(QDataStream& stream) const {
    serialize(*this, stream); // Automatic serialization
}
```

### 4.3 Contracts (Proposed)

**Current Pattern:**
```cpp
// app/logicelement.h - Manual assertions
bool LogicElement::inputValue(const int index) const {
    Q_ASSERT(index >= 0 && index < m_inputValues.size());
    return m_inputValues.at(index);
}
```

**C++26 Potential:**
```cpp
// app/logicelement.h - Built-in contracts
class LogicElement {
public:
    bool inputValue(const int index) const
        pre: index >= 0 && index < m_inputValues.size()
        post r: r == m_inputValues[index]
    {
        return m_inputValues[index];
    }
    
    void updateLogic()
        pre: isValid()
        post: allOutputsValid()
    {
        if (!updateInputs()) return;
        computeOutputs();
    }
    
    void connectPredecessor(int inputIndex, LogicElement* pred, int outputIndex)
        pre: inputIndex >= 0 && inputIndex < inputSize()
        pre: pred != nullptr
        pre: outputIndex >= 0 && outputIndex < pred->outputSize()
        post: m_inputPairs[inputIndex].logic == pred
    {
        m_inputPairs[inputIndex] = {pred, outputIndex};
    }
};
```

### 4.4 Network/Async Extensions (Potential)

**Current Pattern:**
```cpp
// Future: Plugin system with network capabilities
void downloadPlugin(const QString& url) {
    // Manual Qt networking
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));
    // Complex callback handling...
}
```

**C++26 Potential:**
```cpp
#include <experimental/network>
#include <experimental/async>

// Future networking integration
std::task<PluginData> downloadPlugin(const std::string& url) {
    using namespace std::net;
    
    auto client = http_client{};
    auto response = co_await client.get(url);
    
    if (response.status_code() != http_status::ok) {
        throw PluginDownloadError{response.status_code()};
    }
    
    auto pluginData = co_await parsePluginData(response.body());
    co_return pluginData;
}

// Usage with structured concurrency
std::task<void> updateAllPlugins() {
    auto downloadTasks = std::vector<std::task<PluginData>>{};
    
    for (const auto& plugin : installedPlugins()) {
        if (plugin.hasUpdate()) {
            downloadTasks.push_back(downloadPlugin(plugin.updateUrl()));
        }
    }
    
    // Wait for all downloads concurrently
    auto results = co_await std::when_all(std::move(downloadTasks));
    
    for (auto& result : results) {
        installPlugin(co_await result);
    }
}
```

### 4.5 Extended `constexpr` and Compile-Time Features

**Current Pattern:**
```cpp
// app/elementtype.h - Runtime type information
ElementType nextElementType(ElementType current) {
    // Runtime computation
    return static_cast<ElementType>(static_cast<int>(current) + 1);
}
```

**C++26 Potential:**
```cpp
// app/element_types.h - Expanded compile-time capabilities
constexpr std::array ELEMENT_TYPES = {
    ElementType::And, ElementType::Or, ElementType::Not,
    // ... all types
};

constexpr ElementType nextElementType(ElementType current) {
    auto it = std::ranges::find(ELEMENT_TYPES, current);
    if (it != ELEMENT_TYPES.end() && std::next(it) != ELEMENT_TYPES.end()) {
        return *std::next(it);
    }
    return ElementType::Unknown;
}

// Compile-time element validation
consteval bool isValidElementCombination(std::span<const ElementType> types) {
    // Complex compile-time validation logic
    for (auto type : types) {
        if (!isElementTypeSupported(type)) return false;
        if (hasIncompatibleNeighbor(type, types)) return false;
    }
    return true;
}

// Usage:
template<ElementType... Types>
class Circuit {
    static_assert(isValidElementCombination({Types...}), 
                  "Invalid element combination in circuit");
    // ...
};
```

## Phase 5: Container and Algorithm Modernization

### 3.1 Replace Qt Containers with Standard Containers

**Current Pattern:**
```cpp
// Heavy Qt container usage throughout
QVector<GraphicElement*> m_elements;
QHash<QNEPort*, QString> m_varMap;  
QMap<QString, QVariant> properties;
```

**Modernized Pattern:**
```cpp
// app/containers.h - Modern container aliases
#include <vector>
#include <unordered_map>
#include <map>

// Type aliases for consistency
template<typename T>
using Vector = std::vector<T>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename K, typename V> 
using OrderedMap = std::map<K, V>;

// Usage:
Vector<GraphicElement*> m_elements;
HashMap<QNEPort*, std::string> m_varMap;
OrderedMap<std::string, std::variant<int, double, std::string>> properties;
```

### 3.2 Algorithm Modernization

**Current Pattern:**
```cpp
// Manual loops for common operations
QVector<GraphicElement*> validElements;
for (auto* element : allElements) {
    if (element->isValid()) {
        validElements.append(element);
    }
}
```

**C++20 Modernized:**
```cpp
#include <algorithm>
#include <ranges>

// Functional approach
auto validElements = allElements 
                   | std::views::filter(&GraphicElement::isValid)
                   | std::ranges::to<std::vector>();

// Or with traditional algorithms + lambda
std::vector<GraphicElement*> validElements;
std::ranges::copy_if(allElements, std::back_inserter(validElements),
                     [](const auto* elem) { return elem->isValid(); });
```

## Phase 4: Memory Management Improvements

### 4.1 Smart Pointer Consistency

**Current Pattern:**
```cpp
// Mixed ownership patterns
GraphicElement* createElement() { return new And(); } // Raw pointer
std::shared_ptr<LogicElement> createLogic() { /* ... */ } // Smart pointer
```

**Modernized Pattern:**
```cpp
// Consistent smart pointer usage
template<ElementType Type>
std::unique_ptr<GraphicElement> createElement() {
    return std::make_unique<ConcreteElement<Type>>();
}

// Factory with smart pointers
class ElementFactory {
public:
    template<ElementType Type>
    static auto createBoth() -> std::pair<std::unique_ptr<GraphicElement>, 
                                         std::shared_ptr<LogicElement>> {
        auto graphic = std::make_unique<ConcreteGraphicElement<Type>>();
        auto logic = std::make_shared<ConcreteLogicElement<Type>>(graphic->inputSize());
        return {std::move(graphic), logic};
    }
};
```

### 4.2 RAII for Resource Management

**Current Pattern:**
```cpp
// Manual resource management
void loadFile(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;
    // ... processing
    file.close(); // Manual cleanup
}
```

**Modernized Pattern:**
```cpp
// RAII with scope guards
#include <memory>

template<typename T, typename Deleter>
class ScopeGuard {
    T resource;
    Deleter deleter;
public:
    ScopeGuard(T&& r, Deleter d) : resource(std::move(r)), deleter(d) {}
    ~ScopeGuard() { deleter(resource); }
    T& get() { return resource; }
};

template<typename T, typename Deleter>
auto make_scope_guard(T&& resource, Deleter deleter) {
    return ScopeGuard<T, Deleter>(std::forward<T>(resource), deleter);
}

// Usage:
void loadFile(const QString& fileName) {
    auto file = make_scope_guard(QFile(fileName), [](QFile& f) { 
        if (f.isOpen()) f.close(); 
    });
    
    if (!file.get().open(QIODevice::ReadOnly)) return;
    // Automatic cleanup on scope exit
}
```

## Implementation Strategy

### Phase 1: C++17 Foundation (Weeks 1-4)
- [ ] Add structured bindings for map/container iterations
- [ ] Implement `std::optional` for factory functions and error handling
- [ ] Add `std::variant` for unified input element types
- [ ] Use `if constexpr` for template specializations
- [ ] Introduce `std::string_view` for performance-critical string operations

### Phase 2: C++20 Core Modernization (Weeks 5-8)  
- [ ] Implement concepts for type safety constraints
- [ ] Add ranges-based algorithms for container operations
- [ ] Introduce `std::format` for string formatting
- [ ] Add compile-time computations with `consteval`
- [ ] Consider coroutines for async file loading

### Phase 3: C++23 Advanced Features (Weeks 9-12)
- [ ] Replace exception throwing with `std::expected` for better error handling
- [ ] Implement `std::mdspan` for multi-dimensional truth tables
- [ ] Add `std::print`/`std::println` for modern debugging
- [ ] Use `std::flat_map`/`std::flat_set` for performance-critical containers
- [ ] Implement `std::generator` for lazy element traversal
- [ ] Add deducing `this` for CRTP simplification

### Phase 4: C++26 Future-Proofing (Weeks 13-16)
- [ ] **Experimental/Preparation Phase**: Monitor C++26 standard development
- [ ] Prepare infrastructure for pattern matching (when available)
- [ ] Design reflection-ready code structures for automatic serialization
- [ ] Plan contracts integration for better debugging
- [ ] Research async/networking libraries for future plugin system
- [ ] Expand compile-time validation capabilities

### Phase 5: Testing and Validation (Weeks 17-20)
- [ ] Comprehensive testing of all modernized code
- [ ] Performance benchmarking across C++17/20/23 features
- [ ] Memory usage analysis and optimization
- [ ] Backward compatibility verification
- [ ] Compiler compatibility testing (GCC, Clang, MSVC)

### Phase 6: Integration and Documentation (Weeks 21-24)
- [ ] Qt integration refinement and compatibility testing
- [ ] Modern C++ best practices documentation
- [ ] Developer guides for new patterns
- [ ] Migration notes for contributors
- [ ] Performance analysis reports

## Expected Benefits

### 🚀 Performance Improvements
- **C++17**: 5-15% improvement with structured bindings and `string_view`
- **C++20**: 15-25% faster startup with `consteval` and compile-time computations  
- **C++20**: 10-30% faster collection operations with ranges and concepts
- **C++23**: 20-40% better cache performance with `std::flat_map`/`std::flat_set`
- **C++23**: Memory efficiency improvements with lazy evaluation (`std::generator`)
- **C++26**: Potential 30-50% reduction in boilerplate with reflection and pattern matching

### 🔧 Code Quality by Standard
- **C++17**: Better null safety with `std::optional`, cleaner type-safe unions with `std::variant`
- **C++20**: Compile-time error prevention with concepts, functional programming with ranges
- **C++23**: Superior error handling with `std::expected`, multi-dimensional data with `std::mdspan`
- **C++26**: Automatic serialization with reflection, bulletproof APIs with contracts

### 📚 Developer Experience Evolution
- **Better IDE support**: Each standard version improves tooling capabilities
- **Enhanced debugging**: 
  - C++17: Cleaner variable names with structured bindings
  - C++20: Better template error messages with concepts  
  - C++23: Type-safe debugging with `std::print`
  - C++26: Automatic debugging info with reflection
- **Reduced boilerplate**:
  - C++17: Less verbose iterations and error handling
  - C++20: Functional-style algorithms eliminate manual loops
  - C++23: Deducing `this` simplifies CRTP patterns
  - C++26: Reflection eliminates manual enum-to-string conversions

### 🎯 Feature-Specific Benefits

**C++17 Benefits:**
- **Structured Bindings**: Eliminate iterator complexity in map operations
- **std::optional**: Make null cases explicit, reduce crashes
- **std::variant**: Type-safe input element handling
- **if constexpr**: Eliminate SFINAE complexity

**C++20 Benefits:** 
- **Concepts**: Prevent template instantiation errors at compile-time
- **Ranges**: Functional programming reduces bugs and improves readability
- **std::format**: Type-safe string formatting prevents buffer overflows
- **Coroutines**: Non-blocking file I/O improves UI responsiveness

**C++23 Benefits:**
- **std::expected**: Explicit error handling eliminates hidden exceptions
- **std::mdspan**: Clean multi-dimensional array access for truth tables
- **std::flat_map**: 2-5x faster lookups for frequently accessed data
- **std::generator**: Memory-efficient large circuit traversal

**C++26 Future Benefits:**
- **Pattern matching**: 50-70% reduction in switch statement boilerplate
- **Reflection**: Automatic serialization eliminates manual save/load code
- **Contracts**: Runtime validation becomes compile-time guaranteed
- **Networking**: Native async networking for plugin downloads

## Compatibility Considerations

### Qt Integration
- Maintain Qt containers for UI-related code where Qt expects them
- Provide conversion utilities between Qt and standard containers
- Keep Qt-specific functionality (signals/slots, meta-object system)

### Incremental Migration
- Phase implementation allows gradual adoption
- Maintain backward compatibility throughout migration
- Extensive testing at each phase prevents regressions
- Rollback capability if issues arise

## C++ Standards Adoption Timeline

### Immediate (Available Now)
- **C++17**: Fully supported by GCC 7+, Clang 5+, MSVC 2017
- **C++20**: Widely supported by GCC 10+, Clang 10+, MSVC 2019 v16.11+
- **Implementation**: Start with C++17/C++20 features immediately

### Short-term (2025-2026)
- **C++23**: Recently finalized (2023), compiler support maturing
  - GCC 11+ partial support, GCC 13+ comprehensive
  - Clang 15+ partial support, Clang 17+ comprehensive  
  - MSVC 2022+ growing support
- **Implementation**: Begin experimenting with stable C++23 features

### Medium-term (2026-2027)
- **C++23**: Full compiler support across all major compilers
- **C++26**: Standard under development, key features being finalized
- **Implementation**: Production-ready C++23, experimental C++26 preparation

### Long-term (2027+)
- **C++26**: Standard finalized and compiler support beginning
- **Implementation**: Begin adopting early C++26 features, prepare for full migration

## Standards Migration Strategy

### Incremental Adoption Approach
```cpp
// Use feature detection for gradual adoption
#if __cpp_lib_expected >= 202202L
    // Use std::expected for error handling
    std::expected<void, FileError> saveFile();
#else
    // Fallback to current exception-based approach
    void saveFile(); // may throw
#endif

#if __cpp_lib_flat_map >= 202207L
    std::flat_map<ElementType, Properties> m_cache;
#else
    std::map<ElementType, Properties> m_cache;
#endif
```

### Compiler Compatibility Matrix
| Feature | GCC | Clang | MSVC | Status |
|---------|-----|-------|------|---------|
| **C++17 Complete** | 7+ | 5+ | 2017 | ✅ Ready |
| **C++20 Complete** | 10+ | 10+ | 2019 16.11+ | ✅ Ready |
| **C++23 std::expected** | 12+ | 16+ | 2022 17.5+ | ⚠️ Checking |
| **C++23 std::flat_map** | 13+ | 17+ | 2022 17.8+ | ⚠️ Limited |
| **C++23 std::print** | 14+ | 18+ | 2022 17.9+ | 🟡 Emerging |
| **C++26 Pattern Matching** | TBD | TBD | TBD | 🔴 Proposed |
| **C++26 Reflection** | TBD | TBD | TBD | 🔴 Proposed |

### Feature Adoption Priorities

**Phase 1 (Immediate - High Impact, Low Risk)**
1. Structured bindings - universally supported
2. std::optional - prevents crashes
3. if constexpr - eliminates SFINAE complexity
4. Concepts - better error messages

**Phase 2 (Short-term - Medium Impact, Medium Risk)**
1. Ranges - major productivity improvement
2. std::format - type safety + performance  
3. std::expected - better error handling than exceptions
4. std::flat_map - performance critical paths only

**Phase 3 (Medium-term - High Impact, Higher Risk)**
1. std::generator - memory efficiency for large circuits
2. std::mdspan - cleaner multi-dimensional data
3. Deducing this - CRTP simplification
4. std::print - debugging improvements

**Phase 4 (Long-term - Transformative, Experimental)**
1. Pattern matching - eliminate switch statement boilerplate
2. Reflection - automatic serialization 
3. Contracts - compile-time API guarantees
4. Networking TS - plugin system foundation

This comprehensive modernization plan positions wiRedPanda as a cutting-edge C++ codebase that demonstrates best practices across multiple language standards, from immediately available C++17/C++20 features to forward-looking C++26 capabilities, while maintaining practical compatibility and Qt integration throughout the transition.