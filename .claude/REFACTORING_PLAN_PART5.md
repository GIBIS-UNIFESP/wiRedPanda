# ⚡ **PHASE 5: Advanced Error Handling & Recovery**

**Duration**: 2 days | **Risk**: Low | **Impact**: Medium

### **5.1 Context Managers & Retry Logic** (Day 12)

**Objective**: Replace repetitive try/catch patterns with sophisticated error handling that provides better debugging information and automatic recovery capabilities.

**Tasks**:
```python
# Create: framework/error_handling.py
import time
import logging
import functools
from contextlib import contextmanager
from typing import Callable, TypeVar, Optional, Tuple, Any, List
from dataclasses import dataclass
from enum import Enum

# Configure logging for error handling
logger = logging.getLogger('cpu_validation')

T = TypeVar('T')

class ErrorSeverity(Enum):
    """Classification of error severity levels."""
    INFO = "info"
    WARNING = "warning"
    ERROR = "error"
    CRITICAL = "critical"

class ErrorCategory(Enum):
    """Categories of errors that can occur."""
    CIRCUIT_CREATION = "circuit_creation"
    ELEMENT_CREATION = "element_creation"
    CONNECTION_FAILURE = "connection_failure"
    SIMULATION_ERROR = "simulation_error"
    VALIDATION_ERROR = "validation_error"
    CONFIGURATION_ERROR = "configuration_error"
    BRIDGE_COMMUNICATION = "bridge_communication"

@dataclass
class ErrorDetails:
    """Detailed information about an error."""
    category: ErrorCategory
    severity: ErrorSeverity
    message: str
    operation_name: str
    timestamp: float
    context: Optional[Dict[str, Any]] = None
    suggestion: Optional[str] = None
    recoverable: bool = True

class CircuitOperationError(Exception):
    """Base exception for circuit operation failures."""
    def __init__(self, message: str, details: Optional[ErrorDetails] = None):
        super().__init__(message)
        self.details = details

class ElementCreationError(CircuitOperationError):
    """Raised when element creation fails."""
    pass

class ConnectionError(CircuitOperationError):
    """Raised when connection creation fails."""
    pass

class SimulationError(CircuitOperationError):
    """Raised when simulation operations fail."""
    pass

class ErrorRecorder:
    """Records and tracks errors across validation sessions."""
    
    def __init__(self):
        self.errors: List[ErrorDetails] = []
        self.warnings: List[ErrorDetails] = []
        self.operation_failures: Dict[str, int] = {}
        self.current_operation: Optional[str] = None
    
    def record_error(self, details: ErrorDetails) -> None:
        """Record an error with full details."""
        if details.severity in [ErrorSeverity.ERROR, ErrorSeverity.CRITICAL]:
            self.errors.append(details)
        else:
            self.warnings.append(details)
        
        # Track operation failure counts
        if details.operation_name:
            self.operation_failures[details.operation_name] = (
                self.operation_failures.get(details.operation_name, 0) + 1
            )
        
        # Log the error
        log_level = {
            ErrorSeverity.INFO: logging.INFO,
            ErrorSeverity.WARNING: logging.WARNING,
            ErrorSeverity.ERROR: logging.ERROR,
            ErrorSeverity.CRITICAL: logging.CRITICAL
        }[details.severity]
        
        logger.log(log_level, f"[{details.category.value}] {details.message}", 
                   extra={'operation': details.operation_name, 'context': details.context})
    
    def get_error_summary(self) -> Dict[str, Any]:
        """Get summary of all recorded errors."""
        return {
            'total_errors': len(self.errors),
            'total_warnings': len(self.warnings),
            'error_categories': {cat.value: len([e for e in self.errors if e.category == cat]) 
                               for cat in ErrorCategory},
            'most_failed_operations': sorted(self.operation_failures.items(), 
                                           key=lambda x: x[1], reverse=True)[:5]
        }
    
    def has_critical_errors(self) -> bool:
        """Check if any critical errors were recorded."""
        return any(e.severity == ErrorSeverity.CRITICAL for e in self.errors)

@contextmanager
def circuit_operation(validator: 'CPUValidationFramework', operation_name: str, 
                     error_category: ErrorCategory = ErrorCategory.CIRCUIT_CREATION):
    """Context manager for circuit operations with automatic error handling."""
    old_operation = getattr(validator, 'current_operation', None)
    validator.current_operation = operation_name
    
    start_time = time.time()
    
    try:
        logger.info(f"Starting operation: {operation_name}")
        yield validator
        
        elapsed = time.time() - start_time
        logger.info(f"Completed operation: {operation_name} in {elapsed:.3f}s")
        
    except WiredPandaError as e:
        elapsed = time.time() - start_time
        
        details = ErrorDetails(
            category=error_category,
            severity=ErrorSeverity.ERROR,
            message=f"WiredPanda error in {operation_name}: {e}",
            operation_name=operation_name,
            timestamp=time.time(),
            context={'elapsed_time': elapsed, 'exception_type': type(e).__name__},
            suggestion=_get_suggestion_for_error(error_category, str(e)),
            recoverable=_is_recoverable_error(str(e))
        )
        
        if hasattr(validator, 'error_recorder'):
            validator.error_recorder.record_error(details)
        
        raise CircuitOperationError(f"Failed to {operation_name}: {e}", details) from e
        
    except Exception as e:
        elapsed = time.time() - start_time
        
        details = ErrorDetails(
            category=ErrorCategory.VALIDATION_ERROR,
            severity=ErrorSeverity.CRITICAL,
            message=f"Unexpected error in {operation_name}: {e}",
            operation_name=operation_name,
            timestamp=time.time(),
            context={'elapsed_time': elapsed, 'exception_type': type(e).__name__},
            recoverable=False
        )
        
        if hasattr(validator, 'error_recorder'):
            validator.error_recorder.record_error(details)
        
        logger.critical(f"Critical error in '{operation_name}': {e}")
        raise
        
    finally:
        validator.current_operation = old_operation

def _get_suggestion_for_error(category: ErrorCategory, error_message: str) -> Optional[str]:
    """Provide helpful suggestions based on error type and message."""
    suggestions = {
        ErrorCategory.ELEMENT_CREATION: {
            'invalid element type': 'Check that element type is supported (And, Or, Not, Xor, etc.)',
            'position': 'Verify element positions are within valid bounds',
            'label': 'Ensure element labels are unique and non-empty'
        },
        ErrorCategory.CONNECTION_FAILURE: {
            'invalid port': 'Check that source and target ports exist and are valid',
            'same element': 'Cannot connect element to itself',
            'already connected': 'Port may already be connected to another element'
        },
        ErrorCategory.SIMULATION_ERROR: {
            'not started': 'Make sure simulation is started before setting inputs',
            'invalid input': 'Verify input element IDs are valid and correspond to InputButton elements'
        }
    }
    
    category_suggestions = suggestions.get(category, {})
    for key, suggestion in category_suggestions.items():
        if key.lower() in error_message.lower():
            return suggestion
    
    return None

def _is_recoverable_error(error_message: str) -> bool:
    """Determine if an error is potentially recoverable."""
    non_recoverable_patterns = [
        'bridge not connected',
        'wiRedPanda not running',
        'invalid configuration',
        'memory allocation failed'
    ]
    
    return not any(pattern in error_message.lower() for pattern in non_recoverable_patterns)

class RetryPolicy:
    """Configurable retry policy with exponential backoff."""
    
    def __init__(self, max_attempts: int = 3, backoff_factor: float = 1.5, 
                 base_delay: float = 0.5, max_delay: float = 10.0,
                 retry_exceptions: Tuple = (WiredPandaError,)):
        self.max_attempts = max_attempts
        self.backoff_factor = backoff_factor
        self.base_delay = base_delay
        self.max_delay = max_delay
        self.retry_exceptions = retry_exceptions
    
    def execute_with_retry(self, operation: Callable[[], T], operation_name: str,
                          context: Optional[Dict[str, Any]] = None) -> T:
        """Execute operation with exponential backoff retry."""
        last_exception = None
        
        for attempt in range(1, self.max_attempts + 1):
            try:
                logger.debug(f"Attempt {attempt}/{self.max_attempts} for {operation_name}")
                return operation()
                
            except self.retry_exceptions as e:
                last_exception = e
                
                if attempt == self.max_attempts:
                    logger.error(f"All {self.max_attempts} attempts failed for {operation_name}")
                    break
                
                # Calculate delay with exponential backoff
                delay = min(self.base_delay * (self.backoff_factor ** (attempt - 1)), self.max_delay)
                
                logger.warning(f"Attempt {attempt} failed for {operation_name}: {e}. "
                             f"Retrying in {delay:.1f}s...")
                
                time.sleep(delay)
            
            except Exception as e:
                # Non-retryable exception
                logger.error(f"Non-retryable exception in {operation_name}: {e}")
                raise
        
        # All retries exhausted
        raise last_exception

    def __call__(self, operation_name: Optional[str] = None):
        """Decorator version of retry policy."""
        def decorator(func: Callable[..., T]) -> Callable[..., T]:
            @functools.wraps(func)
            def wrapper(*args, **kwargs) -> T:
                name = operation_name or func.__name__
                return self.execute_with_retry(lambda: func(*args, **kwargs), name)
            return wrapper
        return decorator

# Pre-configured retry policies
STANDARD_RETRY = RetryPolicy(max_attempts=3, backoff_factor=1.5, base_delay=0.5)
AGGRESSIVE_RETRY = RetryPolicy(max_attempts=5, backoff_factor=2.0, base_delay=0.2)
GENTLE_RETRY = RetryPolicy(max_attempts=2, backoff_factor=1.2, base_delay=1.0)

class ErrorRecoveryManager:
    """Manages error recovery strategies."""
    
    def __init__(self, validator: 'CPUValidationFramework'):
        self.validator = validator
        self.recovery_strategies: Dict[ErrorCategory, Callable] = {
            ErrorCategory.ELEMENT_CREATION: self._recover_from_element_error,
            ErrorCategory.CONNECTION_FAILURE: self._recover_from_connection_error,
            ErrorCategory.SIMULATION_ERROR: self._recover_from_simulation_error,
        }
    
    def attempt_recovery(self, error: CircuitOperationError) -> bool:
        """Attempt to recover from a circuit operation error."""
        if not error.details or not error.details.recoverable:
            return False
        
        recovery_func = self.recovery_strategies.get(error.details.category)
        if not recovery_func:
            return False
        
        try:
            logger.info(f"Attempting recovery for {error.details.category.value} error")
            return recovery_func(error)
        except Exception as e:
            logger.warning(f"Recovery attempt failed: {e}")
            return False
    
    def _recover_from_element_error(self, error: CircuitOperationError) -> bool:
        """Try to recover from element creation errors."""
        if 'position' in str(error).lower():
            # Try creating element at different positions
            logger.info("Attempting element recovery with position adjustment")
            # Implementation would try alternative positions
            return True  # Placeholder
        
        if 'label' in str(error).lower():
            # Try with auto-generated unique label
            logger.info("Attempting element recovery with unique label")
            return True  # Placeholder
        
        return False
    
    def _recover_from_connection_error(self, error: CircuitOperationError) -> bool:
        """Try to recover from connection errors."""
        if 'port' in str(error).lower():
            # Try different port numbers
            logger.info("Attempting connection recovery with alternative ports")
            return True  # Placeholder
        
        return False
    
    def _recover_from_simulation_error(self, error: CircuitOperationError) -> bool:
        """Try to recover from simulation errors."""
        if 'not started' in str(error).lower():
            # Try to restart simulation
            logger.info("Attempting simulation recovery by restarting")
            try:
                self.validator.start_simulation()
                return True
            except:
                return False
        
        return False
```

### **5.2 Integration with Base Framework** (Day 13)

**Objective**: Integrate the advanced error handling system into the base validation framework and existing validators.

**Tasks**:
```python
# Update: framework/base_validator.py
from .error_handling import (
    ErrorRecorder, RetryPolicy, ErrorRecoveryManager, circuit_operation,
    CircuitOperationError, ErrorCategory, STANDARD_RETRY
)

class CPUValidationFramework:
    def __init__(self, level_name: str, config_path: Optional[str] = None):
        # ... existing initialization ...
        
        # Initialize error handling
        self.error_recorder = ErrorRecorder()
        self.retry_policy = STANDARD_RETRY
        self.recovery_manager = ErrorRecoveryManager(self)
        self.current_operation: Optional[str] = None
        
        # Configure logging
        logging.basicConfig(level=logging.INFO, 
                          format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        self.logger = logging.getLogger(f'cpu_validation.{level_name.lower()}')
    
    def create_element_with_recovery(self, element_type: str, x: float, y: float, 
                                   label: str = "") -> Optional[int]:
        """Create element with error handling and recovery."""
        operation_name = f"create_{element_type}_{label or 'unlabeled'}"
        
        def create_operation():
            with circuit_operation(self, operation_name, ErrorCategory.ELEMENT_CREATION):
                return self.create_element(element_type, x, y, label)
        
        try:
            return self.retry_policy.execute_with_retry(create_operation, operation_name)
        except CircuitOperationError as e:
            # Attempt recovery
            if self.recovery_manager.attempt_recovery(e):
                try:
                    return self.retry_policy.execute_with_retry(create_operation, f"{operation_name}_recovery")
                except CircuitOperationError:
                    pass  # Recovery failed
            
            self.logger.error(f"Failed to create element {element_type} after retries and recovery")
            return None
    
    def connect_elements_with_recovery(self, source_id: int, source_port: int,
                                     target_id: int, target_port: int) -> bool:
        """Connect elements with error handling and recovery."""
        operation_name = f"connect_{source_id}:{source_port}_to_{target_id}:{target_port}"
        
        def connect_operation():
            with circuit_operation(self, operation_name, ErrorCategory.CONNECTION_FAILURE):
                return self.connect_elements(source_id, source_port, target_id, target_port)
        
        try:
            return self.retry_policy.execute_with_retry(connect_operation, operation_name)
        except CircuitOperationError as e:
            if self.recovery_manager.attempt_recovery(e):
                try:
                    return self.retry_policy.execute_with_retry(connect_operation, f"{operation_name}_recovery")
                except CircuitOperationError:
                    pass
            
            self.logger.error(f"Failed to connect elements after retries and recovery")
            return False
    
    def start_simulation_with_recovery(self) -> bool:
        """Start simulation with error handling."""
        operation_name = "start_simulation"
        
        def start_operation():
            with circuit_operation(self, operation_name, ErrorCategory.SIMULATION_ERROR):
                return self.start_simulation()
        
        try:
            return self.retry_policy.execute_with_retry(start_operation, operation_name)
        except CircuitOperationError as e:
            if self.recovery_manager.attempt_recovery(e):
                try:
                    return self.retry_policy.execute_with_retry(start_operation, f"{operation_name}_recovery")
                except CircuitOperationError:
                    pass
            
            self.logger.error("Failed to start simulation after retries and recovery")
            return False
    
    def get_error_report(self) -> Dict[str, Any]:
        """Get comprehensive error report for this validation session."""
        summary = self.error_recorder.get_error_summary()
        
        return {
            'session_summary': summary,
            'has_critical_errors': self.error_recorder.has_critical_errors(),
            'recent_errors': self.error_recorder.errors[-10:],  # Last 10 errors
            'recent_warnings': self.error_recorder.warnings[-10:],  # Last 10 warnings
            'suggestions': self._generate_error_suggestions()
        }
    
    def _generate_error_suggestions(self) -> List[str]:
        """Generate suggestions based on error patterns."""
        suggestions = []
        summary = self.error_recorder.get_error_summary()
        
        if summary['error_categories'].get('bridge_communication', 0) > 0:
            suggestions.append("Check that wiRedPanda is running and MCP server is accessible")
        
        if summary['error_categories'].get('element_creation', 0) > 3:
            suggestions.append("Consider adjusting element positioning or reducing circuit complexity")
        
        if summary['error_categories'].get('connection_failure', 0) > 2:
            suggestions.append("Verify element IDs are valid and ports are not already connected")
        
        return suggestions
    
    def cleanup(self) -> None:
        """Enhanced cleanup with error reporting."""
        # Log final error summary
        if self.error_recorder.errors or self.error_recorder.warnings:
            report = self.get_error_report()
            self.logger.info(f"Session completed with {report['session_summary']['total_errors']} errors, "
                           f"{report['session_summary']['total_warnings']} warnings")
            
            if report['has_critical_errors']:
                self.logger.warning("Session had critical errors that may indicate systemic issues")
        
        # Standard cleanup
        if self.bridge:
            self.bridge.stop()
            self.bridge = None
```

**Usage Examples**:
```python
# Update: framework/circuit_builder.py  
class CircuitBuilder:
    def add_inputs(self, *names: str) -> 'CircuitBuilder':
        """Add input elements with advanced error handling."""
        for i, name in enumerate(names):
            pos = self.layout_manager.get_input_position(i)
            
            # Use enhanced error handling
            element_id_value = self.validator.create_element_with_recovery(
                "InputButton", pos.x, pos.y, name)
            
            if element_id_value is None:
                # Enhanced error information
                error_report = self.validator.get_error_report()
                recent_errors = error_report.get('recent_errors', [])
                if recent_errors:
                    last_error = recent_errors[-1]
                    suggestion = last_error.suggestion or "No specific suggestion available"
                    raise ValueError(f"Failed to create input element '{name}': {suggestion}")
                else:
                    raise ValueError(f"Failed to create input element '{name}': Unknown error")
            
            element_id = ElementID.create(element_id_value, "InputButton", name, pos)
            self.elements[name] = element_id
            self.input_names.append(name)
            
        return self
    
    def connect(self, source: str, target: str, source_port: int = 0, target_port: int = 0) -> 'CircuitBuilder':
        """Connect elements with enhanced error handling."""
        if source not in self.elements:
            raise ValueError(f"Source element '{source}' not found")
        if target not in self.elements:
            raise ValueError(f"Target element '{target}' not found")
            
        source_element = self.elements[source]
        target_element = self.elements[target]
        
        # Use enhanced connection method
        success = self.validator.connect_elements_with_recovery(
            source_element.value, source_port,
            target_element.value, target_port
        )
        
        if not success:
            error_report = self.validator.get_error_report()
            recent_errors = error_report.get('recent_errors', [])
            if recent_errors:
                last_error = recent_errors[-1]
                suggestion = last_error.suggestion or "Check element IDs and port numbers"
                raise ValueError(f"Failed to connect {source} to {target}: {suggestion}")
            else:
                raise ValueError(f"Failed to connect {source} to {target}")
        
        # Create connection record
        connection = Connection(
            source=Port(source_element, source_port),
            target=Port(target_element, target_port)
        )
        self.connections.append(connection)
        
        return self

# Example usage in validators
class AdvancedCombinationalValidator(CPUValidationFramework):
    def test_multi_input_gates_with_enhanced_error_handling(self) -> Dict[str, Any]:
        """Test with comprehensive error handling and reporting."""
        try:
            # Run tests with error handling
            results = self.test_multi_input_gates()
            
            # Add error report to results
            error_report = self.get_error_report()
            results['error_report'] = error_report
            
            # Add recommendations based on errors
            if error_report['has_critical_errors']:
                results['status'] = 'completed_with_critical_errors'
                results['recommendations'] = error_report['suggestions']
            elif error_report['session_summary']['total_errors'] > 0:
                results['status'] = 'completed_with_errors'
                results['recommendations'] = error_report['suggestions']
            else:
                results['status'] = 'completed_successfully'
            
            return results
            
        except Exception as e:
            # Capture any unhandled exceptions
            self.logger.critical(f"Unhandled exception in test execution: {e}")
            return {
                'success': False,
                'error': str(e),
                'error_report': self.get_error_report(),
                'status': 'failed'
            }
```

**Testing Advanced Error Handling**:
```python
# Create: tests/test_error_handling.py
import pytest
from framework.error_handling import *
from framework.base_validator import CPUValidationFramework

class TestErrorHandling:
    def test_circuit_operation_context_manager(self):
        """Test circuit operation context manager."""
        validator = CPUValidationFramework("Test")
        
        # Test successful operation
        with circuit_operation(validator, "test_operation"):
            # Simulate successful operation
            pass
        
        assert validator.current_operation is None
    
    def test_retry_policy_success_on_retry(self):
        """Test retry policy succeeds on second attempt."""
        attempts = 0
        
        def flaky_operation():
            nonlocal attempts
            attempts += 1
            if attempts < 2:
                raise WiredPandaError("Temporary failure")
            return "success"
        
        retry_policy = RetryPolicy(max_attempts=3)
        result = retry_policy.execute_with_retry(flaky_operation, "test_op")
        
        assert result == "success"
        assert attempts == 2
    
    def test_error_recorder_tracks_errors(self):
        """Test error recorder functionality."""
        recorder = ErrorRecorder()
        
        error_details = ErrorDetails(
            category=ErrorCategory.ELEMENT_CREATION,
            severity=ErrorSeverity.ERROR,
            message="Test error",
            operation_name="test_operation",
            timestamp=time.time()
        )
        
        recorder.record_error(error_details)
        
        assert len(recorder.errors) == 1
        assert recorder.operation_failures['test_operation'] == 1
```

**Benefits of Enhanced Error Handling**:
1. **Better Debugging**: Detailed error context and suggestions
2. **Automatic Recovery**: Common errors resolved without user intervention  
3. **Retry Logic**: Temporary failures handled transparently
4. **Error Analytics**: Track error patterns to identify systemic issues
5. **Graceful Degradation**: System continues operating despite individual failures