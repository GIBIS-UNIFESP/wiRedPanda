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