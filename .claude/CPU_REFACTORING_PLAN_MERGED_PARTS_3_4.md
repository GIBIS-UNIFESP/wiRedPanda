# ⚙️ **PHASE 3: Configuration-Driven Architecture**

**Duration**: 2-3 days | **Risk**: Medium | **Impact**: High

### **3.1 External Configuration System** (Day 5)

**Objective**: Replace hardcoded test configurations with external YAML files that can be modified without code changes.

**File Structure**:
```
config/
├── level1_config.yaml        # Multi-input gates, compound functions
├── level2_config.yaml        # Adders, subtractors, comparators
├── common_elements.yaml      # Shared gate definitions
├── layout_config.yaml        # Grid positioning settings
└── schemas/
    └── test_config.schema.json  # JSON schema validation
```

**Example Configuration**:
```yaml
# config/level1_config.yaml
metadata:
  level: 1
  name: "Advanced Combinational Logic"
  description: "Validation for advanced combinational logic components"
  version: "1.0.0"

test_suites:
  multi_input_gates:
    description: "Multi-Input Logic Gates Validation"
    enabled: true
    tests:
      - name: "3_input_and"
        type: "combinational_gate"
        description: "3-input AND gate validation"
        inputs: ["A", "B", "C"]
        function: 
          type: "and_gate"
          input_count: 3
        expected_accuracy: 100.0
        circuit_save_path: "level1_3_input_and.panda"
        test_cases: "all_combinations"  # Generate all 2^3 combinations
        layout:
          auto_position: true
          input_column: 0
          gate_column: 2
          output_column: 4
        
      - name: "4_input_or" 
        type: "combinational_gate"
        description: "4-input OR gate validation"
        inputs: ["A", "B", "C", "D"]
        function:
          type: "or_gate" 
          input_count: 4
        expected_accuracy: 100.0
        circuit_save_path: "level1_4_input_or.panda"
        test_cases: 
          type: "sample"  # Use representative sample instead of all combinations
          count: 10       # Number of test cases to generate
          include_corners: true  # Always include corner cases (all 0s, all 1s)

  compound_boolean_functions:
    description: "Complex Boolean Function Validation"
    enabled: true
    tests:
      - name: "aoi22"
        type: "compound_function"
        description: "AND-OR-INVERT: F = NOT((A AND B) OR (C AND D))"
        inputs: ["A", "B", "C", "D"]
        function:
          type: "custom"
          expression: "not((A and B) or (C and D))"
          truth_table_file: "truth_tables/aoi22.csv"  # Optional pre-computed table
        expected_accuracy: 100.0
        circuit_save_path: "level1_aoi22.panda"
        test_cases: "all_combinations"

layout_config:
  grid:
    start_x: 80
    start_y: 100
    element_width: 64
    element_height: 64
    horizontal_spacing: 150
    vertical_spacing: 130
  
  auto_layout:
    enabled: true
    prevent_overlaps: true
    optimize_connections: true
    
performance_config:
  max_test_cases_per_circuit: 32  # Limit for performance
  parallel_execution: false       # Future: parallel test execution
  timeout_seconds: 300            # Per-test timeout
  
error_handling:
  retry_attempts: 3
  retry_delay: 1.0
  fail_fast: false  # Continue testing even if individual tests fail
```

```yaml
# config/level2_config.yaml
metadata:
  level: 2
  name: "Arithmetic Building Blocks"
  description: "Validation for arithmetic circuit components"
  version: "1.0.0"

test_suites:
  basic_adders:
    description: "Half and Full Adder Validation"
    enabled: true
    tests:
      - name: "half_adder"
        type: "arithmetic_circuit"
        description: "Half Adder: Sum = A XOR B, Carry = A AND B"
        inputs: ["A", "B"]
        outputs: ["Sum", "Carry"]
        function:
          type: "half_adder"
          expected_behavior:
            - {A: false, B: false, Sum: false, Carry: false}
            - {A: false, B: true, Sum: true, Carry: false}
            - {A: true, B: false, Sum: true, Carry: false}
            - {A: true, B: true, Sum: false, Carry: true}
        circuit_save_path: "level2_half_adder.panda"

      - name: "full_adder"
        type: "arithmetic_circuit" 
        description: "Full Adder with carry input"
        inputs: ["A", "B", "Cin"]
        outputs: ["Sum", "Carry"]
        function:
          type: "full_adder"
          # Use programmatic generation instead of listing all 8 combinations
        expected_accuracy: 100.0
        circuit_save_path: "level2_full_adder.panda"

  multi_bit_adders:
    description: "Multi-bit Ripple Carry Adders"
    enabled: true
    tests:
      - name: "4_bit_adder"
        type: "multi_bit_arithmetic"
        description: "4-bit Ripple Carry Adder"
        bit_width: 4
        inputs: ["A[3:0]", "B[3:0]", "Cin"]  # Bus notation
        outputs: ["Sum[3:0]", "Cout"]
        function:
          type: "ripple_carry_adder"
          bit_width: 4
        test_cases:
          type: "comprehensive"
          # Test specific patterns for multi-bit arithmetic
          patterns:
            - "corner_cases"      # 0+0, max+max, etc.
            - "carry_propagation" # Cases that test carry chains
            - "random_sampling"   # Random test cases
          count: 50
        expected_accuracy: 100.0
        circuit_save_path: "level2_4_bit_adder.panda"
```

```json
// config/schemas/test_config.schema.json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "https://cpu-validation.com/schemas/test-config.json",
  "title": "CPU Validation Test Configuration",
  "description": "Schema for CPU validation test configuration files",
  "type": "object",
  "required": ["metadata", "test_suites"],
  "properties": {
    "metadata": {
      "type": "object",
      "required": ["level", "name", "description", "version"],
      "properties": {
        "level": {"type": "integer", "minimum": 1},
        "name": {"type": "string", "minLength": 1},
        "description": {"type": "string"},
        "version": {"type": "string", "pattern": "^\\d+\\.\\d+\\.\\d+$"}
      }
    },
    "test_suites": {
      "type": "object",
      "patternProperties": {
        "^[a-zA-Z_][a-zA-Z0-9_]*$": {
          "$ref": "#/definitions/test_suite"
        }
      }
    },
    "layout_config": {"$ref": "#/definitions/layout_config"},
    "performance_config": {"$ref": "#/definitions/performance_config"},
    "error_handling": {"$ref": "#/definitions/error_handling"}
  },
  "definitions": {
    "test_suite": {
      "type": "object",
      "required": ["description", "enabled", "tests"],
      "properties": {
        "description": {"type": "string"},
        "enabled": {"type": "boolean"},
        "tests": {
          "type": "array",
          "minItems": 1,
          "items": {"$ref": "#/definitions/test_case"}
        }
      }
    },
    "test_case": {
      "type": "object",
      "required": ["name", "type", "description"],
      "properties": {
        "name": {"type": "string", "pattern": "^[a-zA-Z_][a-zA-Z0-9_]*$"},
        "type": {"type": "string", "enum": ["combinational_gate", "arithmetic_circuit", "compound_function"]},
        "description": {"type": "string"},
        "inputs": {"type": "array", "items": {"type": "string"}},
        "outputs": {"type": "array", "items": {"type": "string"}},
        "function": {"$ref": "#/definitions/function_config"},
        "expected_accuracy": {"type": "number", "minimum": 0, "maximum": 100},
        "circuit_save_path": {"type": "string"},
        "test_cases": {"$ref": "#/definitions/test_cases_config"}
      }
    }
  }
}
```

### **3.2 Configuration Parser & Validator** (Day 6)

**Objective**: Create robust configuration parsing with schema validation and error handling.

**Tasks**:
```python
# Create: framework/config_parser.py
import yaml
import json
import jsonschema
from dataclasses import dataclass
from typing import Dict, List, Any, Optional, Union
from pathlib import Path
from enum import Enum

class TestType(Enum):
    COMBINATIONAL_GATE = "combinational_gate"
    ARITHMETIC_CIRCUIT = "arithmetic_circuit"
    COMPOUND_FUNCTION = "compound_function"
    MULTI_BIT_ARITHMETIC = "multi_bit_arithmetic"

class FunctionType(Enum):
    AND_GATE = "and_gate"
    OR_GATE = "or_gate"
    XOR_GATE = "xor_gate"
    HALF_ADDER = "half_adder"
    FULL_ADDER = "full_adder"
    RIPPLE_CARRY_ADDER = "ripple_carry_adder"
    CUSTOM = "custom"

@dataclass
class FunctionConfig:
    """Configuration for circuit function definition."""
    type: FunctionType
    input_count: Optional[int] = None
    bit_width: Optional[int] = None
    expression: Optional[str] = None
    truth_table_file: Optional[str] = None
    expected_behavior: Optional[List[Dict[str, Any]]] = None

@dataclass
class TestCasesConfig:
    """Configuration for test case generation."""
    type: str = "all_combinations"  # "all_combinations", "sample", "comprehensive"
    count: Optional[int] = None
    include_corners: bool = True
    patterns: Optional[List[str]] = None

@dataclass
class LayoutConfig:
    """Configuration for circuit layout."""
    auto_position: bool = True
    input_column: int = 0
    gate_column: int = 2
    output_column: int = 4

@dataclass
class TestConfig:
    """Individual test configuration."""
    name: str
    test_type: TestType
    description: str
    inputs: List[str]
    outputs: Optional[List[str]] = None
    function: Optional[FunctionConfig] = None
    expected_accuracy: float = 100.0
    circuit_save_path: Optional[str] = None
    test_cases: Optional[TestCasesConfig] = None
    layout: Optional[LayoutConfig] = None

@dataclass
class TestSuiteConfig:
    """Test suite configuration."""
    description: str
    enabled: bool
    tests: List[TestConfig]

@dataclass
class MetadataConfig:
    """Configuration metadata."""
    level: int
    name: str
    description: str
    version: str

@dataclass
class PerformanceConfig:
    """Performance-related configuration."""
    max_test_cases_per_circuit: int = 32
    parallel_execution: bool = False
    timeout_seconds: int = 300

@dataclass
class ErrorHandlingConfig:
    """Error handling configuration."""
    retry_attempts: int = 3
    retry_delay: float = 1.0
    fail_fast: bool = False

@dataclass
class GridConfig:
    """Grid layout configuration."""
    start_x: int = 80
    start_y: int = 100
    element_width: int = 64
    element_height: int = 64
    horizontal_spacing: int = 150
    vertical_spacing: int = 130

@dataclass
class AutoLayoutConfig:
    """Auto-layout configuration."""
    enabled: bool = True
    prevent_overlaps: bool = True
    optimize_connections: bool = True

@dataclass
class LayoutConfiguration:
    """Complete layout configuration."""
    grid: GridConfig
    auto_layout: AutoLayoutConfig

@dataclass
class ValidationConfig:
    """Complete validation configuration."""
    metadata: MetadataConfig
    test_suites: Dict[str, TestSuiteConfig]
    layout_config: Optional[LayoutConfiguration] = None
    performance_config: Optional[PerformanceConfig] = None
    error_handling: Optional[ErrorHandlingConfig] = None

class ConfigValidationError(Exception):
    """Raised when configuration validation fails."""
    pass

class ConfigParser:
    """Parser and validator for CPU validation configurations."""
    
    def __init__(self, schema_path: Optional[str] = None):
        """Initialize parser with optional schema validation."""
        self.schema_path = schema_path
        self.schema: Optional[Dict[str, Any]] = None
        
        if schema_path and Path(schema_path).exists():
            with open(schema_path, 'r') as f:
                self.schema = json.load(f)
    
    def parse_config_file(self, config_path: str) -> ValidationConfig:
        """Parse configuration from YAML file."""
        config_file = Path(config_path)
        if not config_file.exists():
            raise FileNotFoundError(f"Configuration file not found: {config_path}")
        
        try:
            with open(config_file, 'r') as f:
                raw_config = yaml.safe_load(f)
        except yaml.YAMLError as e:
            raise ConfigValidationError(f"Invalid YAML in {config_path}: {e}")
        
        # Validate against schema if available
        if self.schema:
            try:
                jsonschema.validate(raw_config, self.schema)
            except jsonschema.ValidationError as e:
                raise ConfigValidationError(f"Configuration validation failed: {e.message}")
        
        return self._parse_config_dict(raw_config)
    
    def _parse_config_dict(self, config: Dict[str, Any]) -> ValidationConfig:
        """Parse configuration dictionary into typed objects."""
        try:
            # Parse metadata
            metadata_dict = config["metadata"]
            metadata = MetadataConfig(
                level=metadata_dict["level"],
                name=metadata_dict["name"],
                description=metadata_dict["description"],
                version=metadata_dict["version"]
            )
            
            # Parse test suites
            test_suites = {}
            for suite_name, suite_config in config["test_suites"].items():
                test_suites[suite_name] = self._parse_test_suite(suite_config)
            
            # Parse optional configurations
            layout_config = None
            if "layout_config" in config:
                layout_config = self._parse_layout_config(config["layout_config"])
            
            performance_config = None
            if "performance_config" in config:
                perf_dict = config["performance_config"]
                performance_config = PerformanceConfig(
                    max_test_cases_per_circuit=perf_dict.get("max_test_cases_per_circuit", 32),
                    parallel_execution=perf_dict.get("parallel_execution", False),
                    timeout_seconds=perf_dict.get("timeout_seconds", 300)
                )
            
            error_handling = None
            if "error_handling" in config:
                error_dict = config["error_handling"]
                error_handling = ErrorHandlingConfig(
                    retry_attempts=error_dict.get("retry_attempts", 3),
                    retry_delay=error_dict.get("retry_delay", 1.0),
                    fail_fast=error_dict.get("fail_fast", False)
                )
            
            return ValidationConfig(
                metadata=metadata,
                test_suites=test_suites,
                layout_config=layout_config,
                performance_config=performance_config,
                error_handling=error_handling
            )
            
        except (KeyError, TypeError, ValueError) as e:
            raise ConfigValidationError(f"Invalid configuration structure: {e}")
    
    def _parse_test_suite(self, suite_config: Dict[str, Any]) -> TestSuiteConfig:
        """Parse individual test suite configuration."""
        tests = []
        for test_config in suite_config["tests"]:
            tests.append(self._parse_test_config(test_config))
        
        return TestSuiteConfig(
            description=suite_config["description"],
            enabled=suite_config["enabled"],
            tests=tests
        )
    
    def _parse_test_config(self, test_config: Dict[str, Any]) -> TestConfig:
        """Parse individual test configuration."""
        # Parse function config if present
        function = None
        if "function" in test_config:
            function = self._parse_function_config(test_config["function"])
        
        # Parse test cases config
        test_cases = None
        if "test_cases" in test_config:
            test_cases = self._parse_test_cases_config(test_config["test_cases"])
        
        # Parse layout config
        layout = None
        if "layout" in test_config:
            layout_dict = test_config["layout"]
            layout = LayoutConfig(
                auto_position=layout_dict.get("auto_position", True),
                input_column=layout_dict.get("input_column", 0),
                gate_column=layout_dict.get("gate_column", 2),
                output_column=layout_dict.get("output_column", 4)
            )
        
        return TestConfig(
            name=test_config["name"],
            test_type=TestType(test_config["type"]),
            description=test_config["description"],
            inputs=test_config["inputs"],
            outputs=test_config.get("outputs"),
            function=function,
            expected_accuracy=test_config.get("expected_accuracy", 100.0),
            circuit_save_path=test_config.get("circuit_save_path"),
            test_cases=test_cases,
            layout=layout
        )
    
    def _parse_function_config(self, func_config: Dict[str, Any]) -> FunctionConfig:
        """Parse function configuration."""
        return FunctionConfig(
            type=FunctionType(func_config["type"]),
            input_count=func_config.get("input_count"),
            bit_width=func_config.get("bit_width"),
            expression=func_config.get("expression"),
            truth_table_file=func_config.get("truth_table_file"),
            expected_behavior=func_config.get("expected_behavior")
        )
    
    def _parse_test_cases_config(self, cases_config: Union[str, Dict[str, Any]]) -> TestCasesConfig:
        """Parse test cases configuration."""
        if isinstance(cases_config, str):
            return TestCasesConfig(type=cases_config)
        
        return TestCasesConfig(
            type=cases_config.get("type", "all_combinations"),
            count=cases_config.get("count"),
            include_corners=cases_config.get("include_corners", True),
            patterns=cases_config.get("patterns")
        )
    
    def _parse_layout_config(self, layout_config: Dict[str, Any]) -> LayoutConfiguration:
        """Parse layout configuration."""
        grid_dict = layout_config.get("grid", {})
        grid = GridConfig(
            start_x=grid_dict.get("start_x", 80),
            start_y=grid_dict.get("start_y", 100),
            element_width=grid_dict.get("element_width", 64),
            element_height=grid_dict.get("element_height", 64),
            horizontal_spacing=grid_dict.get("horizontal_spacing", 150),
            vertical_spacing=grid_dict.get("vertical_spacing", 130)
        )
        
        auto_dict = layout_config.get("auto_layout", {})
        auto_layout = AutoLayoutConfig(
            enabled=auto_dict.get("enabled", True),
            prevent_overlaps=auto_dict.get("prevent_overlaps", True),
            optimize_connections=auto_dict.get("optimize_connections", True)
        )
        
        return LayoutConfiguration(grid=grid, auto_layout=auto_layout)

# Utility functions for configuration management
def load_config(config_path: str, schema_path: Optional[str] = None) -> ValidationConfig:
    """Convenience function to load and parse configuration."""
    parser = ConfigParser(schema_path)
    return parser.parse_config_file(config_path)

def validate_config_file(config_path: str, schema_path: str) -> bool:
    """Validate configuration file against schema."""
    try:
        load_config(config_path, schema_path)
        return True
    except (ConfigValidationError, FileNotFoundError):
        return False
```

### **3.3 Configuration Integration** (Day 7)

**Objective**: Integrate configuration system into existing validators while maintaining backward compatibility.

**Migration Strategy**:
1. Keep existing hardcoded configs as fallback
2. Add configuration loading capability
3. Gradually migrate tests to use external configs
4. Remove hardcoded configs once verified

**Tasks**:
```python
# Update: framework/base_validator.py
from .config_parser import ValidationConfig, load_config
from .domain_types import GridConfig

class CPUValidationFramework:
    def __init__(self, level_name: str, config_path: Optional[str] = None):
        # ... existing initialization ...
        
        # Load configuration
        self.config: Optional[ValidationConfig] = None
        if config_path:
            try:
                self.config = load_config(config_path, "config/schemas/test_config.schema.json")
                print(f"✅ Loaded configuration: {self.config.metadata.name}")
            except Exception as e:
                print(f"⚠️  Failed to load config {config_path}: {e}")
                print("   Falling back to hardcoded configuration")
        
        # Set up layout from config or defaults
        if self.config and self.config.layout_config:
            grid = self.config.layout_config.grid
            self.GRID_START_X = grid.start_x
            self.GRID_START_Y = grid.start_y
            self.MIN_HORIZONTAL_SPACING = grid.horizontal_spacing
            self.MIN_VERTICAL_SPACING = grid.vertical_spacing
    
    def get_test_suite_config(self, suite_name: str) -> Optional[TestSuiteConfig]:
        """Get configuration for a specific test suite."""
        if self.config and suite_name in self.config.test_suites:
            return self.config.test_suites[suite_name]
        return None
    
    def is_test_enabled(self, suite_name: str) -> bool:
        """Check if a test suite is enabled in configuration."""
        suite_config = self.get_test_suite_config(suite_name)
        return suite_config.enabled if suite_config else True

# Update existing validators
# In: validators/combinational.py (updated from cpu_level_1_advanced_combinational.py)
from framework.base_validator import CPUValidationFramework
from framework.config_parser import TestConfig, FunctionType

class AdvancedCombinationalValidator(CPUValidationFramework):
    def __init__(self, config_path: Optional[str] = None):
        super().__init__("Level 1: Advanced Combinational Logic", 
                        config_path or "config/level1_config.yaml")
    
    def test_multi_input_gates(self) -> Dict[str, Any]:
        """Test multi-input gates using configuration."""
        if not self.is_test_enabled("multi_input_gates"):
            return {"status": "skipped", "reason": "Test disabled in configuration"}
        
        suite_config = self.get_test_suite_config("multi_input_gates")
        if not suite_config:
            # Fallback to hardcoded configuration
            return self._test_multi_input_gates_hardcoded()
        
        test_results = {}
        all_tests_passed = True
        
        for test_config in suite_config.tests:
            print(f"  Testing {test_config.name}...")
            
            result = self._run_configured_test(test_config)
            test_results[test_config.name] = result
            
            if not result["success"]:
                all_tests_passed = False
            
            # Save circuit if configured
            if test_config.circuit_save_path:
                self.save_circuit(test_config.circuit_save_path)
                print(f"  💾 Circuit saved: {test_config.circuit_save_path}")
        
        return {
            "success": all_tests_passed,
            "description": suite_config.description,
            "results": test_results
        }
    
    def _run_configured_test(self, test_config: TestConfig) -> Dict[str, Any]:
        """Run a single test based on configuration."""
        if not test_config.function:
            return {"success": False, "error": "No function configuration provided"}
        
        # Create new circuit
        if not self.create_new_circuit():
            return {"success": False, "error": "Failed to create circuit"}
        
        # Build circuit based on configuration
        circuit_result = self._build_circuit_from_config(test_config)
        if not circuit_result:
            return {"success": False, "error": "Failed to build circuit"}
        
        # Generate test cases based on configuration
        test_cases = self._generate_test_cases_from_config(test_config)
        
        # Execute tests
        return self._execute_test_cases(circuit_result, test_cases, test_config)
    
    def _build_circuit_from_config(self, test_config: TestConfig) -> Optional[CircuitResult]:
        """Build circuit based on test configuration."""
        if test_config.function.type == FunctionType.AND_GATE:
            return self._build_and_gate_circuit(test_config)
        elif test_config.function.type == FunctionType.OR_GATE:
            return self._build_or_gate_circuit(test_config)
        # Add more circuit types as needed
        
        return None
    
    def _build_and_gate_circuit(self, test_config: TestConfig) -> Optional[CircuitResult]:
        """Build AND gate circuit from configuration."""
        input_count = len(test_config.inputs)
        
        # Create inputs
        input_ids = []
        for i, input_name in enumerate(test_config.inputs):
            pos = self._get_input_position(i)
            input_id = self.create_element("InputButton", pos[0], pos[1], input_name)
            if input_id is None:
                return None
            input_ids.append(input_id)
        
        # Create AND gate (use cascaded approach for >2 inputs)
        if input_count == 2:
            gate_x, gate_y = self._get_grid_position(2, 0)
            gate_id = self.create_element("And", gate_x, gate_y, "AND")
        else:
            # For >2 inputs, create cascade of 2-input AND gates
            gate_id = self._create_multi_input_and_cascade(input_ids)
        
        if gate_id is None:
            return None
        
        # Create output
        output_x, output_y = self._get_output_position(4, 0)
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        if output_id is None:
            return None
        
        # Connect circuit
        if input_count == 2:
            # Direct connections for 2-input gate
            if not self.connect_elements(input_ids[0], 0, gate_id, 0):
                return None
            if not self.connect_elements(input_ids[1], 0, gate_id, 1):
                return None
        
        # Connect gate to output
        if not self.connect_elements(gate_id, 0, output_id, 0):
            return None
        
        return CircuitResult(
            layout=CircuitLayout(
                elements={"gate": ElementID.create(gate_id, "And"), 
                         "output": ElementID.create(output_id, "Led")},
                connections=[],
                input_elements=test_config.inputs,
                output_elements=["OUT"]
            ),
            input_ids=[ElementID.create(id, "InputButton") for id in input_ids],
            output_ids=[ElementID.create(output_id, "Led")],
            creation_time=time.time(),
            element_count=len(input_ids) + 1 + 1,  # inputs + gate + output
            connection_count=len(input_ids) + 1    # input connections + output connection
        )
```

**Testing Integration**:
```bash
# Commands to test configuration integration
python3 -c "
from framework.config_parser import load_config
config = load_config('config/level1_config.yaml', 'config/schemas/test_config.schema.json')
print(f'Loaded {len(config.test_suites)} test suites')
for name, suite in config.test_suites.items():
    print(f'  {name}: {len(suite.tests)} tests, enabled={suite.enabled}')
"

# Test with configuration
python3 validators/combinational.py  # Should use config/level1_config.yaml

# Test backward compatibility (no config file)
mv config/level1_config.yaml config/level1_config.yaml.backup
python3 validators/combinational.py  # Should fallback to hardcoded config
mv config/level1_config.yaml.backup config/level1_config.yaml
```

# 🔧 **PHASE 4: Circuit Builder DSL**

**Duration**: 3-4 days | **Risk**: Medium | **Impact**: Very High

### **4.1 Fluent Circuit Builder Core** (Day 8-9)

**Objective**: Create a Domain-Specific Language for circuit construction that reduces verbose boilerplate from 50+ lines to 5-10 lines while improving readability and maintainability.

**Tasks**:
```python
# Create: framework/circuit_builder.py
from typing import Dict, List, Optional, Callable, Any, Union
from dataclasses import dataclass, field
from .domain_types import ElementID, Position, Connection, Port, CircuitLayout, CircuitResult
from .base_validator import CPUValidationFramework
import time

@dataclass
class AutoLayoutManager:
    """Manages automatic positioning of circuit elements."""
    
    def __init__(self, grid_config: GridConfig):
        self.grid = grid_config
        self.input_positions: List[Position] = []
        self.gate_positions: List[Position] = []
        self.output_positions: List[Position] = []
        self.current_gate_row = 0
        self.current_gate_col = 2  # Start gates at column 2
        
    def get_input_position(self, index: int) -> Position:
        """Calculate input position with collision avoidance."""
        pos = self.grid.get_position(0, index)
        self.input_positions.append(pos)
        return pos
        
    def get_next_gate_position(self, gate_type: str) -> Position:
        """Smart gate positioning based on circuit flow."""
        pos = self.grid.get_position(self.current_gate_col, self.current_gate_row)
        
        # Update position for next gate
        self.current_gate_row += 1
        if self.current_gate_row > 3:  # Max 4 gates per column
            self.current_gate_row = 0
            self.current_gate_col += 1
            
        self.gate_positions.append(pos)
        return pos
        
    def get_output_position(self, index: int) -> Position:
        """Calculate output position."""
        # Place outputs to the right of all gates
        output_col = max(self.current_gate_col + 1, 4)  # Minimum column 4
        pos = self.grid.get_position(output_col, index)
        self.output_positions.append(pos)
        return pos
        
    def optimize_layout(self) -> None:
        """Post-build layout optimization for better visual appeal."""
        # Future: Implement layout optimization algorithms
        pass

@dataclass
class GateSpec:
    """Specification for creating a gate with automatic connections."""
    gate_type: str
    name: str
    inputs: Optional[List[str]] = None
    auto_connect: bool = True

class CircuitBuilder:
    """Fluent interface for building circuits declaratively."""
    
    def __init__(self, validator: CPUValidationFramework):
        self.validator = validator
        self.elements: Dict[str, ElementID] = {}
        self.connections: List[Connection] = []
        self.input_names: List[str] = []
        self.output_names: List[str] = []
        self.layout_manager = AutoLayoutManager(
            GridConfig(
                start_x=validator.GRID_START_X,
                start_y=validator.GRID_START_Y,
                horizontal_spacing=validator.MIN_HORIZONTAL_SPACING,
                vertical_spacing=validator.MIN_VERTICAL_SPACING
            )
        )
        
    def add_inputs(self, *names: str) -> 'CircuitBuilder':
        """Add input elements with automatic positioning."""
        for i, name in enumerate(names):
            pos = self.layout_manager.get_input_position(i)
            element_id_value = self.validator.create_element("InputButton", pos.x, pos.y, name)
            if element_id_value is None:
                raise ValueError(f"Failed to create input element '{name}'")
            
            element_id = ElementID.create(element_id_value, "InputButton", name, pos)
            self.elements[name] = element_id
            self.input_names.append(name)
            
        return self
        
    def add_gate(self, gate_type: str, name: str, inputs: Optional[List[str]] = None) -> 'CircuitBuilder':
        """Add logic gate with automatic positioning and optional auto-connection."""
        pos = self.layout_manager.get_next_gate_position(gate_type)
        element_id_value = self.validator.create_element(gate_type, pos.x, pos.y, name)
        if element_id_value is None:
            raise ValueError(f"Failed to create gate '{name}' of type '{gate_type}'")
        
        element_id = ElementID.create(element_id_value, gate_type, name, pos)
        self.elements[name] = element_id
        
        # Auto-connect if inputs specified
        if inputs:
            self._auto_connect_inputs(name, inputs)
            
        return self
        
    def add_gates(self, gates: List[Union[GateSpec, Tuple[str, str, List[str]]]]) -> 'CircuitBuilder':
        """Add multiple gates at once for more compact circuit definitions."""
        for gate in gates:
            if isinstance(gate, tuple):
                gate_type, name, inputs = gate
                self.add_gate(gate_type, name, inputs)
            else:
                self.add_gate(gate.gate_type, gate.name, gate.inputs)
        return self
        
    def connect(self, source: str, target: str, source_port: int = 0, target_port: int = 0) -> 'CircuitBuilder':
        """Connect two elements by name."""
        if source not in self.elements:
            raise ValueError(f"Source element '{source}' not found")
        if target not in self.elements:
            raise ValueError(f"Target element '{target}' not found")
            
        source_element = self.elements[source]
        target_element = self.elements[target]
        
        connection = Connection(
            source=Port(source_element, source_port),
            target=Port(target_element, target_port)
        )
        
        # Validate and create physical connection
        connection.validate()
        success = self.validator.connect_elements(
            source_element.value, source_port,
            target_element.value, target_port
        )
        
        if not success:
            raise ValueError(f"Failed to connect {source} to {target}")
            
        self.connections.append(connection)
        return self
        
    def connect_chain(self, *element_names: str) -> 'CircuitBuilder':
        """Connect elements in a chain: A -> B -> C -> D."""
        for i in range(len(element_names) - 1):
            self.connect(element_names[i], element_names[i + 1])
        return self
        
    def connect_fan_out(self, source: str, *targets: str) -> 'CircuitBuilder':
        """Connect one source to multiple targets."""
        for target in targets:
            self.connect(source, target)
        return self
        
    def connect_fan_in(self, *sources: str, target: str) -> 'CircuitBuilder':
        """Connect multiple sources to one target (for multi-input gates)."""
        for i, source in enumerate(sources):
            self.connect(source, target, target_port=i)
        return self
        
    def add_outputs(self, *names: str) -> 'CircuitBuilder':
        """Add output LEDs with automatic positioning."""
        for i, name in enumerate(names):
            pos = self.layout_manager.get_output_position(i)
            element_id_value = self.validator.create_element("Led", pos.x, pos.y, name)
            if element_id_value is None:
                raise ValueError(f"Failed to create output element '{name}'")
            
            element_id = ElementID.create(element_id_value, "Led", name, pos)
            self.elements[name] = element_id
            self.output_names.append(name)
            
        return self
        
    def with_layout_optimization(self) -> 'CircuitBuilder':
        """Enable layout optimization (applied during build)."""
        # Future: Enable various layout optimization algorithms
        return self
        
    def build(self) -> CircuitResult:
        """Finalize circuit and return result."""
        # Optimize layout if requested
        self.layout_manager.optimize_layout()
        
        # Create circuit layout
        layout = CircuitLayout(
            elements=self.elements,
            connections=self.connections,
            input_elements=self.input_names,
            output_elements=self.output_names
        )
        
        # Collect input/output IDs
        input_ids = [self.elements[name] for name in self.input_names]
        output_ids = [self.elements[name] for name in self.output_names]
        
        return CircuitResult(
            layout=layout,
            input_ids=input_ids,
            output_ids=output_ids,
            creation_time=time.time(),
            element_count=len(self.elements),
            connection_count=len(self.connections)
        )
        
    def _auto_connect_inputs(self, gate_name: str, input_names: List[str]) -> None:
        """Automatically connect specified inputs to gate ports."""
        for i, input_name in enumerate(input_names):
            if input_name in self.elements:
                self.connect(input_name, gate_name, target_port=i)
            else:
                raise ValueError(f"Input '{input_name}' not found for auto-connection to '{gate_name}'")
                
    def _get_input_names(self) -> List[str]:
        """Get list of input element names."""
        return self.input_names
        
    def _get_output_names(self) -> List[str]:
        """Get list of output element names."""
        return self.output_names

# Convenience functions for common circuit patterns
def create_simple_gate_circuit(validator: CPUValidationFramework, 
                              gate_type: str, 
                              inputs: List[str],
                              output_name: str = "OUT") -> CircuitResult:
    """Create a simple gate circuit with specified inputs."""
    return (CircuitBuilder(validator)
            .add_inputs(*inputs)
            .add_gate(gate_type, "GATE", inputs)
            .add_outputs(output_name)
            .connect("GATE", output_name)
            .build())

def create_compound_circuit(validator: CPUValidationFramework,
                          inputs: List[str],
                          gates: List[Tuple[str, str, List[str]]],
                          outputs: List[str],
                          final_connections: List[Tuple[str, str]]) -> CircuitResult:
    """Create a compound circuit with multiple gates and connections."""
    builder = (CircuitBuilder(validator)
              .add_inputs(*inputs)
              .add_gates(gates)
              .add_outputs(*outputs))
    
    # Add final connections
    for source, target in final_connections:
        builder.connect(source, target)
    
    return builder.build()
```

### **4.2 Advanced DSL Features** (Day 9)

**Objective**: Add advanced DSL features for complex circuits and reusable patterns.

**Tasks**:
```python
# Extend: framework/circuit_builder.py

class CircuitTemplate:
    """Template for creating reusable circuit patterns."""
    
    def __init__(self, name: str, 
                 input_spec: List[str], 
                 output_spec: List[str],
                 builder_func: Callable[['CircuitBuilder'], 'CircuitBuilder']):
        self.name = name
        self.input_spec = input_spec
        self.output_spec = output_spec
        self.builder_func = builder_func
    
    def create_circuit(self, validator: CPUValidationFramework, 
                      input_names: Optional[List[str]] = None,
                      output_names: Optional[List[str]] = None) -> CircuitResult:
        """Create circuit from template."""
        actual_inputs = input_names or self.input_spec
        actual_outputs = output_names or self.output_spec
        
        if len(actual_inputs) != len(self.input_spec):
            raise ValueError(f"Expected {len(self.input_spec)} inputs, got {len(actual_inputs)}")
        if len(actual_outputs) != len(self.output_spec):
            raise ValueError(f"Expected {len(self.output_spec)} outputs, got {len(actual_outputs)}")
        
        builder = CircuitBuilder(validator).add_inputs(*actual_inputs)
        builder = self.builder_func(builder)
        return builder.add_outputs(*actual_outputs).build()

# Pre-defined circuit templates
class StandardCircuits:
    """Library of standard circuit templates."""
    
    @staticmethod
    def half_adder() -> CircuitTemplate:
        """Half adder template: Sum = A XOR B, Carry = A AND B."""
        def build_half_adder(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gate("Xor", "XOR1", ["A", "B"])
                    .add_gate("And", "AND1", ["A", "B"])
                    .add_outputs("Sum", "Carry")
                    .connect("XOR1", "Sum")
                    .connect("AND1", "Carry"))
        
        return CircuitTemplate("half_adder", ["A", "B"], ["Sum", "Carry"], build_half_adder)
    
    @staticmethod  
    def full_adder() -> CircuitTemplate:
        """Full adder template: Sum = A XOR B XOR Cin, Carry = (A AND B) OR (Cin AND (A XOR B))."""
        def build_full_adder(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gates([
                        ("Xor", "XOR1", ["A", "B"]),
                        ("Xor", "XOR2", ["XOR1", "Cin"]),
                        ("And", "AND1", ["A", "B"]),
                        ("And", "AND2", ["XOR1", "Cin"]),
                        ("Or", "OR1", ["AND1", "AND2"])
                    ])
                    .add_outputs("Sum", "Carry")
                    .connect("XOR2", "Sum")
                    .connect("OR1", "Carry"))
        
        return CircuitTemplate("full_adder", ["A", "B", "Cin"], ["Sum", "Carry"], build_full_adder)
    
    @staticmethod
    def multiplexer_2to1() -> CircuitTemplate:
        """2-to-1 multiplexer template."""
        def build_mux2to1(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gate("Not", "NOT_S", ["S"])
                    .add_gates([
                        ("And", "AND0", ["D0", "NOT_S"]),
                        ("And", "AND1", ["D1", "S"]),
                        ("Or", "OR1", ["AND0", "AND1"])
                    ])
                    .add_outputs("OUT")
                    .connect("OR1", "OUT"))
        
        return CircuitTemplate("mux2to1", ["D0", "D1", "S"], ["OUT"], build_mux2to1)
    
    @staticmethod
    def decoder_2to4() -> CircuitTemplate:
        """2-to-4 decoder with enable."""
        def build_decoder(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gates([
                        ("Not", "NOT_A", ["A"]),
                        ("Not", "NOT_B", ["B"]),
                        ("And", "AND0", ["NOT_A", "NOT_B", "EN"]),
                        ("And", "AND1", ["A", "NOT_B", "EN"]),
                        ("And", "AND2", ["NOT_A", "B", "EN"]),
                        ("And", "AND3", ["A", "B", "EN"])
                    ])
                    .add_outputs("OUT0", "OUT1", "OUT2", "OUT3")
                    .connect_fan_out("AND0", "OUT0")
                    .connect_fan_out("AND1", "OUT1")
                    .connect_fan_out("AND2", "OUT2")
                    .connect_fan_out("AND3", "OUT3"))
        
        return CircuitTemplate("decoder_2to4", ["A", "B", "EN"], ["OUT0", "OUT1", "OUT2", "OUT3"], build_decoder)

# Extended CircuitBuilder with template support
class CircuitBuilder:
    # ... existing methods ...
    
    def use_template(self, template: CircuitTemplate, 
                    input_mapping: Optional[Dict[str, str]] = None,
                    output_mapping: Optional[Dict[str, str]] = None) -> 'CircuitBuilder':
        """Incorporate a circuit template into the current builder."""
        # This is a complex feature for future implementation
        # Would need to merge template circuits into current circuit
        raise NotImplementedError("Template incorporation not yet implemented")
    
    def create_bus(self, name_prefix: str, width: int, element_type: str = "InputButton") -> 'CircuitBuilder':
        """Create a bus of elements (e.g., 4-bit input bus)."""
        names = [f"{name_prefix}[{i}]" for i in range(width)]
        
        if element_type == "InputButton":
            return self.add_inputs(*names)
        elif element_type == "Led":
            return self.add_outputs(*names)
        else:
            # Create gates in a bus configuration
            for i, name in enumerate(names):
                pos = self.layout_manager.get_next_gate_position(element_type)
                element_id_value = self.validator.create_element(element_type, pos.x, pos.y, name)
                if element_id_value is None:
                    raise ValueError(f"Failed to create bus element '{name}'")
                element_id = ElementID.create(element_id_value, element_type, name, pos)
                self.elements[name] = element_id
            
        return self
    
    def connect_bus_to_bus(self, source_prefix: str, target_prefix: str, width: int) -> 'CircuitBuilder':
        """Connect two buses bit by bit."""
        for i in range(width):
            source_name = f"{source_prefix}[{i}]"
            target_name = f"{target_prefix}[{i}]"
            self.connect(source_name, target_name)
        return self
    
    def add_intermediate_node(self, name: str) -> 'CircuitBuilder':
        """Add an intermediate connection node (useful for complex routing)."""
        # Use a buffer (two NOT gates) as an intermediate node
        pos1 = self.layout_manager.get_next_gate_position("Not")
        pos2 = self.layout_manager.get_next_gate_position("Not")
        
        not1_id = self.validator.create_element("Not", pos1.x, pos1.y, f"{name}_NOT1")
        not2_id = self.validator.create_element("Not", pos2.x, pos2.y, f"{name}_NOT2")
        
        if not1_id is None or not2_id is None:
            raise ValueError(f"Failed to create intermediate node '{name}'")
        
        # Connect the two NOTs to form a buffer
        self.validator.connect_elements(not1_id, 0, not2_id, 0)
        
        # Store the output of the buffer as the named node
        element_id = ElementID.create(not2_id, "Not", name, pos2)
        self.elements[name] = element_id
        
        return self
```

### **4.3 DSL Integration & Migration** (Day 10-11)

**Objective**: Systematically migrate existing verbose circuit creation methods to use the new DSL.

**Before & After Examples**:
```python
# BEFORE: 50+ lines (from existing code)
def _create_full_adder_circuit(self) -> Optional[Tuple[InputIDs, OutputIDs]]:
    """Create full adder circuit."""
    # Create inputs with proper grid positioning
    a_x, a_y = self._get_input_position(0)
    a_id = self.create_element("InputButton", a_x, a_y, "A")
    b_x, b_y = self._get_input_position(1)
    b_id = self.create_element("InputButton", b_x, b_y, "B")
    cin_x, cin_y = self._get_input_position(2)
    cin_id = self.create_element("InputButton", cin_x, cin_y, "Cin")

    if not all([a_id, b_id, cin_id]):
        return None

    # Create XOR gates for sum calculation with proper positioning
    xor1_x, xor1_y = self._get_grid_position(1, 0)
    xor1_id = self.create_element("Xor", xor1_x, xor1_y, "XOR1")  # A XOR B
    xor2_x, xor2_y = self._get_grid_position(2, 0)
    xor2_id = self.create_element("Xor", xor2_x, xor2_y, "XOR2")  # (A XOR B) XOR Cin

    # Create AND gates for carry calculation with proper positioning
    and1_x, and1_y = self._get_grid_position(1, 1)
    and1_id = self.create_element("And", and1_x, and1_y, "AND1")  # A AND B
    and2_x, and2_y = self._get_grid_position(2, 1)
    and2_id = self.create_element(
        "And", and2_x, and2_y, "AND2"
    )  # Cin AND (A XOR B)
    or_x, or_y = self._get_grid_position(3, 1)
    or_id = self.create_element("Or", or_x, or_y, "OR")  # Final carry

    gates = [xor1_id, xor2_id, and1_id, and2_id, or_id]
    if not all(gates):
        return None

    # Type assertions after validation
    assert all(gate_id is not None for gate_id in gates)
    assert all(x is not None for x in [a_id, b_id, cin_id])

    # Connect sum logic
    sum_connections = [
        (cast(int, a_id), 0, cast(int, xor1_id), 0),
        (cast(int, b_id), 0, cast(int, xor1_id), 1),
        (cast(int, xor1_id), 0, cast(int, xor2_id), 0),
        (cast(int, cin_id), 0, cast(int, xor2_id), 1),
    ]

    for source_id, source_port, target_id, target_port in sum_connections:
        if not self.connect_elements(source_id, source_port, target_id, target_port):
            return None

    # Connect carry logic
    carry_connections = [
        (cast(int, a_id), 0, cast(int, and1_id), 0),
        (cast(int, b_id), 0, cast(int, and1_id), 1),
        (cast(int, cin_id), 0, cast(int, and2_id), 0),
        (cast(int, xor1_id), 0, cast(int, and2_id), 1),
        (cast(int, and1_id), 0, cast(int, or_id), 0),
        (cast(int, and2_id), 0, cast(int, or_id), 1),
    ]

    for source_id, source_port, target_id, target_port in carry_connections:
        if not self.connect_elements(source_id, source_port, target_id, target_port):
            return None

    # Create outputs with proper grid positioning
    sum_out_x, sum_out_y = self._get_grid_position(3, 0)
    sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, "SUM")
    carry_out_x, carry_out_y = self._get_grid_position(4, 1)
    carry_out_id = self.create_element("Led", carry_out_x, carry_out_y, "CARRY")

    if not all([sum_out_id, carry_out_id]):
        return None

    # Type assertions after validation
    assert all(x is not None for x in [sum_out_id, carry_out_id])

    # Connect outputs
    if not self.connect_elements(cast(int, xor2_id), 0, cast(int, sum_out_id), 0):
        return None
    if not self.connect_elements(cast(int, or_id), 0, cast(int, carry_out_id), 0):
        return None

    return [cast(int, a_id), cast(int, b_id), cast(int, cin_id)], [
        cast(int, sum_out_id),
        cast(int, carry_out_id),
    ]

# AFTER: 8 lines using DSL
def _create_full_adder_circuit(self) -> CircuitResult:
    """Create full adder circuit using DSL."""
    return (CircuitBuilder(self)
            .add_inputs("A", "B", "Cin")
            .add_gates([
                ("Xor", "XOR1", ["A", "B"]),
                ("Xor", "XOR2", ["XOR1", "Cin"]),
                ("And", "AND1", ["A", "B"]),
                ("And", "AND2", ["XOR1", "Cin"]),
                ("Or", "OR1", ["AND1", "AND2"])
            ])
            .add_outputs("Sum", "Carry")
            .connect("XOR2", "Sum")
            .connect("OR1", "Carry")
            .build())

# EVEN SHORTER: Using templates
def _create_full_adder_circuit(self) -> CircuitResult:
    """Create full adder circuit using template."""
    return StandardCircuits.full_adder().create_circuit(self)
```

**Migration Plan**:
1. **Phase 4a (Day 10)**: Migrate simple circuits (AND, OR, XOR gates)
2. **Phase 4b (Day 11)**: Migrate complex circuits (full adder, multiplexer, decoder)

**Implementation Tasks**:
```python
# Update: validators/combinational.py
class AdvancedCombinationalValidator(CPUValidationFramework):
    # ... existing methods ...
    
    def _create_3_input_and_circuit(self) -> CircuitResult:
        """Create 3-input AND gate circuit - MIGRATED TO DSL."""
        return create_simple_gate_circuit(self, "And", ["A", "B", "C"])
    
    def _create_4_input_or_circuit(self) -> CircuitResult:
        """Create 4-input OR gate circuit - MIGRATED TO DSL."""
        # For >2 inputs, need cascade approach
        return (CircuitBuilder(self)
                .add_inputs("A", "B", "C", "D")
                .add_gate("Or", "OR1", ["A", "B"])
                .add_gate("Or", "OR2", ["C", "D"])  
                .add_gate("Or", "OR_FINAL", ["OR1", "OR2"])
                .add_outputs("OUT")
                .connect("OR_FINAL", "OUT")
                .build())
    
    def _create_aoi22_circuit(self) -> CircuitResult:
        """Create AOI22 circuit: F = NOT((A AND B) OR (C AND D)) - MIGRATED TO DSL."""
        return (CircuitBuilder(self)
                .add_inputs("A", "B", "C", "D")
                .add_gates([
                    ("And", "AND1", ["A", "B"]),
                    ("And", "AND2", ["C", "D"]),
                    ("Or", "OR1", ["AND1", "AND2"]),
                    ("Not", "NOT1", ["OR1"])
                ])
                .add_outputs("OUT")
                .connect("NOT1", "OUT")
                .build())
    
    def _create_decoder_2_to_4_circuit(self) -> CircuitResult:
        """Create 2-to-4 decoder - MIGRATED TO DSL."""
        return StandardCircuits.decoder_2to4().create_circuit(
            self, ["A", "B", "EN"], ["OUT0", "OUT1", "OUT2", "OUT3"])
    
    def _create_mux_4_to_1_circuit(self) -> CircuitResult:
        """Create 4-to-1 multiplexer - MIGRATED TO DSL."""
        return (CircuitBuilder(self)
                .add_inputs("D0", "D1", "D2", "D3", "S0", "S1")
                .add_gates([
                    ("Not", "NOT_S0", ["S0"]),
                    ("Not", "NOT_S1", ["S1"]),
                    # Select decoding
                    ("And", "SEL0", ["NOT_S1", "NOT_S0"]),  # S1=0, S0=0
                    ("And", "SEL1", ["NOT_S1", "S0"]),     # S1=0, S0=1
                    ("And", "SEL2", ["S1", "NOT_S0"]),     # S1=1, S0=0
                    ("And", "SEL3", ["S1", "S0"]),        # S1=1, S0=1
                    # Data gating
                    ("And", "GATE0", ["D0", "SEL0"]),
                    ("And", "GATE1", ["D1", "SEL1"]),
                    ("And", "GATE2", ["D2", "SEL2"]),
                    ("And", "GATE3", ["D3", "SEL3"]),
                    # Final OR
                    ("Or", "OR1", ["GATE0", "GATE1"]),
                    ("Or", "OR2", ["GATE2", "GATE3"]),
                    ("Or", "OR_FINAL", ["OR1", "OR2"])
                ])
                .add_outputs("OUT")
                .connect("OR_FINAL", "OUT")
                .build())

# Update: validators/arithmetic.py  
class ArithmeticBlocksValidator(CPUValidationFramework):
    # ... existing methods ...
    
    def _create_half_adder_circuit(self) -> CircuitResult:
        """Create half adder circuit - MIGRATED TO DSL."""
        return StandardCircuits.half_adder().create_circuit(self)
    
    def _create_full_adder_circuit(self) -> CircuitResult:
        """Create full adder circuit - MIGRATED TO DSL."""  
        return StandardCircuits.full_adder().create_circuit(self)
    
    def _create_4_bit_adder_circuit(self) -> CircuitResult:
        """Create 4-bit ripple carry adder - MIGRATED TO DSL."""
        return (CircuitBuilder(self)
                .create_bus("A", 4, "InputButton")    # A[3:0]
                .create_bus("B", 4, "InputButton")    # B[3:0]
                .add_inputs("Cin")
                # Create 4 full adder stages
                .add_gates([
                    # Stage 0
                    ("Xor", "XOR1_0", ["A[0]", "B[0]"]),
                    ("Xor", "XOR2_0", ["XOR1_0", "Cin"]),
                    ("And", "AND1_0", ["A[0]", "B[0]"]),
                    ("And", "AND2_0", ["XOR1_0", "Cin"]),
                    ("Or", "OR_0", ["AND1_0", "AND2_0"]),
                    # Stage 1  
                    ("Xor", "XOR1_1", ["A[1]", "B[1]"]),
                    ("Xor", "XOR2_1", ["XOR1_1", "OR_0"]),
                    ("And", "AND1_1", ["A[1]", "B[1]"]),
                    ("And", "AND2_1", ["XOR1_1", "OR_0"]),
                    ("Or", "OR_1", ["AND1_1", "AND2_1"]),
                    # Continue for stages 2 and 3...
                ])
                .create_bus("Sum", 4, "Led")         # Sum[3:0]
                .add_outputs("Cout")
                .connect("XOR2_0", "Sum[0]")
                .connect("XOR2_1", "Sum[1]")  
                # Continue connections...
                .build())
```

**Testing Migration**:
```bash
# Test individual migrated circuits
python3 -c "
from validators.combinational import AdvancedCombinationalValidator
validator = AdvancedCombinationalValidator()
validator.create_new_circuit()
circuit = validator._create_full_adder_circuit()
print(f'Created circuit with {circuit.element_count} elements, {circuit.connection_count} connections')
validator.save_circuit('test_dsl_full_adder.panda')
"

# Run full validation to ensure no regressions
python3 validators/combinational.py   # Must still show 4/4 tests passed
python3 validators/arithmetic.py     # Must still show 4/4 tests passed
```

**Success Metrics**:
- Circuit creation code reduced by 80%+ (50+ lines → 5-10 lines)
- Improved readability and maintainability
- Zero functional regressions
- All tests continue to pass with identical results