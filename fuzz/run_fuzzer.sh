#!/bin/bash
# wiRedPanda Fuzzing Script
# Comprehensive automation for security testing with libFuzzer

set -e  # Exit on any error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
FUZZER_BIN="$BUILD_DIR/fuzz_serialization"
SEED_CORPUS_DIR="$SCRIPT_DIR/corpus"          # Original hand-crafted seed files (committed)
WORK_CORPUS_DIR="$SCRIPT_DIR/work_corpus"     # Generated corpus during fuzzing (not committed)
FINDINGS_DIR="$SCRIPT_DIR/findings"
DICT_FILE="$SCRIPT_DIR/serialization.dict"

# Default parameters
DURATION=${FUZZ_DURATION:-60}
JOBS=${FUZZ_JOBS:-1}
TIMEOUT=${FUZZ_TIMEOUT:-30}
MAX_LEN=${FUZZ_MAX_LEN:-65536}
RSS_LIMIT=${FUZZ_RSS_LIMIT:-2048}
WORKERS=${FUZZ_WORKERS:-1}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging function
log() {
    echo -e "${BLUE}[$(date +'%H:%M:%S')]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Help message
show_help() {
    cat << EOF
wiRedPanda Fuzzing Script - Automated Security Testing

USAGE:
    $0 [OPTIONS] [COMMAND]

COMMANDS:
    build       Build fuzzer with optimal settings
    fuzz        Run fuzzing session (default)
    minimize    Minimize crash files to smallest reproducers
    verify      Verify fuzzer setup and test with corpus
    coverage    Generate coverage report
    clean       Clean fuzzing artifacts
    reproduce   Reproduce a specific crash file
    help        Show this help message

OPTIONS:
    -t, --time SECONDS     Fuzzing duration (default: 60)
    -j, --jobs NUMBER      Parallel jobs (default: 1)  
    -w, --workers NUMBER   Worker processes for parallel fuzzing (default: 1)
    -m, --max-len BYTES    Maximum input length (default: 65536)
    -r, --rss-limit MB     RSS memory limit (default: 2048)
    --timeout SECONDS      Per-input timeout (default: 30)
    --dict FILE            Custom dictionary file
    --corpus DIR           Custom working corpus directory (default: work_corpus)
    --findings DIR         Custom findings output directory
    --debug                Enable debug output
    --continuous           Run continuous fuzzing (ignore time limit)

ENVIRONMENT VARIABLES:
    FUZZ_DURATION         Default fuzzing duration in seconds
    FUZZ_JOBS             Default number of parallel jobs
    FUZZ_WORKERS          Default number of worker processes
    FUZZ_MAX_LEN          Default maximum input length
    FUZZ_RSS_LIMIT        Default RSS memory limit in MB
    FUZZ_TIMEOUT          Default per-input timeout

EXAMPLES:
    # Quick 5-minute fuzzing session
    $0 fuzz -t 300
    
    # Intensive parallel fuzzing
    $0 fuzz -t 1800 -w 4 -j 4
    
    # Memory-constrained fuzzing
    $0 fuzz -t 600 -r 1024 -m 32768
    
    # Reproduce specific crash
    $0 reproduce crash-abc123def456
    
    # Generate coverage report
    $0 coverage
    
    # Continuous fuzzing (Ctrl+C to stop)
    $0 fuzz --continuous

FUZZING TARGETS:
    - Binary file format parsing (app/serialization.cpp)
    - Magic header validation (WPCF/WPWF)  
    - Version compatibility handling
    - Qt object deserialization
    - Memory allocation patterns

For detailed documentation, see: fuzz/README.md
EOF
}

# Check if clang is available
check_clang() {
    if ! command -v clang++ &> /dev/null; then
        error "Clang++ not found. Install with: sudo apt install clang"
        return 1
    fi
    
    local clang_version=$(clang++ --version | head -n1)
    log "Using $clang_version"
}

# Build fuzzer with optimal settings
build_fuzzer() {
    log "Building fuzzer with optimal settings..."
    
    cd "$PROJECT_ROOT"
    
    # Clean previous build if it exists
    if [ -d "$BUILD_DIR" ]; then
        log "Cleaning previous build..."
        rm -rf "$BUILD_DIR"
    fi
    
    # Configure with Clang and fuzzing enabled
    log "Configuring CMake..."
    CC=clang CXX=clang++ cmake -B build -G Ninja \
        -DENABLE_FUZZING=ON \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DENABLE_ADDRESS_SANITIZER=ON \
        -DENABLE_UB_SANITIZER=ON
    
    # Build fuzzer target
    log "Building fuzz_serialization target..."
    cmake --build build --target fuzz_serialization -j$(nproc)
    
    if [ ! -f "$FUZZER_BIN" ]; then
        error "Fuzzer binary not found at $FUZZER_BIN"
        return 1
    fi
    
    success "Fuzzer built successfully: $FUZZER_BIN"
}

# Verify fuzzer setup
verify_setup() {
    log "Verifying fuzzer setup..."
    
    # Check if fuzzer binary exists
    if [ ! -f "$FUZZER_BIN" ]; then
        error "Fuzzer binary not found. Run: $0 build"
        return 1
    fi
    
    # Check seed corpus directory
    if [ ! -d "$SEED_CORPUS_DIR" ]; then
        error "Seed corpus directory not found: $SEED_CORPUS_DIR"
        return 1
    fi
    
    local seed_count=$(find "$SEED_CORPUS_DIR" -name "*.panda" | wc -l)
    log "Found $seed_count seed corpus files"
    
    # Test fuzzer with a single input
    log "Testing fuzzer with sample input..."
    local test_file="$SEED_CORPUS_DIR/input.panda"
    if [ -f "$test_file" ]; then
        if timeout 10s "$FUZZER_BIN" "$test_file" &>/dev/null; then
            success "Fuzzer test passed"
        else
            error "Fuzzer test failed"
            return 1
        fi
    else
        warning "No test file found, skipping execution test"
    fi
    
    success "Fuzzer setup verified"
}

# Run fuzzing session
run_fuzzer() {
    log "Starting fuzzing session..."
    
    # Create working directories
    mkdir -p "$FINDINGS_DIR"
    mkdir -p "$WORK_CORPUS_DIR"
    
    # Copy seed corpus to working corpus if empty
    if [ -z "$(ls -A "$WORK_CORPUS_DIR" 2>/dev/null)" ]; then
        log "Initializing working corpus from seed corpus..."
        cp "$SEED_CORPUS_DIR"/*.panda "$WORK_CORPUS_DIR/" 2>/dev/null || true
    fi
    
    # Set Qt environment for headless operation
    export QT_QPA_PLATFORM=offscreen
    export QT_LOGGING_RULES="*=false"
    
    # Build fuzzer arguments
    local args=(
        "$WORK_CORPUS_DIR"          # Working corpus (will be modified)
        "$SEED_CORPUS_DIR"           # Additional seed corpus (read-only)
        "-max_len=$MAX_LEN"
        "-timeout=$TIMEOUT"
        "-rss_limit_mb=$RSS_LIMIT"
        "-print_final_stats=1"
        "-reload=1"
        "-report_slow_units=10"
        "-artifact_prefix=$FINDINGS_DIR/"
    )
    
    # Add duration unless continuous mode
    if [ "$CONTINUOUS" != "1" ]; then
        args+=("-max_total_time=$DURATION")
    fi
    
    # Add dictionary if available
    if [ -f "$DICT_FILE" ]; then
        args+=("-dict=$DICT_FILE")
        log "Using dictionary: $DICT_FILE"
    fi
    
    # Add parallel execution options
    if [ "$WORKERS" -gt 1 ]; then
        args+=("-workers=$WORKERS" "-jobs=$JOBS")
        log "Running with $WORKERS workers and $JOBS jobs"
    fi
    
    log "Fuzzing parameters:"
    log "  Duration: $([ "$CONTINUOUS" = "1" ] && echo "continuous" || echo "${DURATION}s")"
    log "  Max length: $MAX_LEN bytes"
    log "  Timeout: ${TIMEOUT}s per input"
    log "  RSS limit: ${RSS_LIMIT}MB"
    log "  Seed corpus: $SEED_CORPUS_DIR ($(find "$SEED_CORPUS_DIR" -name "*.panda" | wc -l) files)"
    log "  Working corpus: $WORK_CORPUS_DIR ($(find "$WORK_CORPUS_DIR" -name "*.panda" 2>/dev/null | wc -l) files)"
    
    log "Starting fuzzer (Ctrl+C to stop)..."
    
    # Run the fuzzer
    local start_time=$(date +%s)
    "$FUZZER_BIN" "${args[@]}" || {
        local exit_code=$?
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        
        if [ $exit_code -eq 130 ]; then
            log "Fuzzing interrupted by user after ${duration}s"
        else
            error "Fuzzer exited with code $exit_code after ${duration}s"
        fi
    }
    
    # Check for findings
    check_findings
}

# Check for crash findings
check_findings() {
    log "Checking for findings..."
    
    local crashes=$(find "$FINDINGS_DIR" -name "crash-*" -type f 2>/dev/null | wc -l)
    local leaks=$(find "$FINDINGS_DIR" -name "leak-*" -type f 2>/dev/null | wc -l)
    local ooms=$(find "$FINDINGS_DIR" -name "oom-*" -type f 2>/dev/null | wc -l)
    local timeouts=$(find "$FINDINGS_DIR" -name "timeout-*" -type f 2>/dev/null | wc -l)
    
    if [ "$crashes" -gt 0 ]; then
        error "🚨 Found $crashes crash(es)!"
        find "$FINDINGS_DIR" -name "crash-*" -type f | while read -r crash; do
            error "  - $(basename "$crash")"
        done
        log "To reproduce: $0 reproduce <crash-file>"
        log "To minimize: $0 minimize"
        return 1
    fi
    
    if [ "$leaks" -gt 0 ]; then
        warning "Found $leaks memory leak(s)"
    fi
    
    if [ "$ooms" -gt 0 ]; then
        warning "Found $ooms out-of-memory error(s)"
    fi
    
    if [ "$timeouts" -gt 0 ]; then
        warning "Found $timeouts timeout(s)"
    fi
    
    if [ "$crashes" -eq 0 ] && [ "$leaks" -eq 0 ] && [ "$ooms" -eq 0 ] && [ "$timeouts" -eq 0 ]; then
        success "No critical issues found during fuzzing session"
    fi
}

# Minimize crash files
minimize_crashes() {
    log "Minimizing crash files..."
    
    local crashes=$(find "$FINDINGS_DIR" -name "crash-*" -type f 2>/dev/null)
    
    if [ -z "$crashes" ]; then
        log "No crash files found to minimize"
        return 0
    fi
    
    for crash in $crashes; do
        local basename=$(basename "$crash")
        local minimized="$FINDINGS_DIR/minimized-$basename"
        
        log "Minimizing $basename..."
        
        "$FUZZER_BIN" \
            -minimize_crash=1 \
            -runs=10000 \
            -exact_artifact_path="$minimized" \
            "$crash" || {
            warning "Failed to minimize $basename"
            continue
        }
        
        if [ -f "$minimized" ]; then
            local orig_size=$(stat -f%z "$crash" 2>/dev/null || stat -c%s "$crash")
            local min_size=$(stat -f%z "$minimized" 2>/dev/null || stat -c%s "$minimized")
            success "Minimized $basename: $orig_size → $min_size bytes"
        fi
    done
}

# Reproduce a specific crash
reproduce_crash() {
    local crash_file="$1"
    
    if [ -z "$crash_file" ]; then
        error "Usage: $0 reproduce <crash-file>"
        return 1
    fi
    
    # Check if file exists as given or in findings directory
    if [ ! -f "$crash_file" ]; then
        crash_file="$FINDINGS_DIR/$crash_file"
        if [ ! -f "$crash_file" ]; then
            error "Crash file not found: $1"
            return 1
        fi
    fi
    
    log "Reproducing crash: $(basename "$crash_file")"
    log "File size: $(stat -f%z "$crash_file" 2>/dev/null || stat -c%s "$crash_file") bytes"
    
    export QT_QPA_PLATFORM=offscreen
    "$FUZZER_BIN" "$crash_file"
}

# Generate coverage report
generate_coverage() {
    log "Generating coverage report..."
    
    # Build with coverage enabled
    cd "$PROJECT_ROOT"
    CC=clang CXX=clang++ cmake -B build-coverage -G Ninja \
        -DENABLE_COVERAGE=ON \
        -DENABLE_FUZZING=ON \
        -DCMAKE_BUILD_TYPE=Debug
    
    cmake --build build-coverage --target fuzz_serialization
    
    # Create temporary working corpus for coverage
    local temp_corpus="/tmp/coverage_corpus"
    mkdir -p "$temp_corpus"
    cp "$SEED_CORPUS_DIR"/*.panda "$temp_corpus/" 2>/dev/null || true
    
    # Run fuzzer briefly to generate coverage data
    export QT_QPA_PLATFORM=offscreen
    "./build-coverage/fuzz_serialization" "$temp_corpus" -runs=1000 || true
    
    # Clean up temporary corpus
    rm -rf "$temp_corpus"
    
    # Generate coverage report
    if command -v lcov &> /dev/null; then
        lcov --capture --directory build-coverage --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        genhtml coverage.info --output-directory coverage-report
        success "Coverage report generated: coverage-report/index.html"
    else
        warning "lcov not found. Install with: sudo apt install lcov"
    fi
}

# Clean fuzzing artifacts
clean_artifacts() {
    log "Cleaning fuzzing artifacts..."
    
    # Remove findings
    if [ -d "$FINDINGS_DIR" ]; then
        rm -rf "$FINDINGS_DIR"
        log "Removed findings directory"
    fi
    
    # Remove coverage files
    rm -f coverage.info
    rm -rf coverage-report
    
    # Remove any crash files in project root
    find "$PROJECT_ROOT" -name "crash-*" -type f -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "leak-*" -type f -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "oom-*" -type f -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "timeout-*" -type f -delete 2>/dev/null || true
    
    success "Cleaned fuzzing artifacts"
}

# Parse command line arguments
parse_args() {
    COMMAND="fuzz"
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            -t|--time)
                DURATION="$2"
                shift 2
                ;;
            -j|--jobs)
                JOBS="$2"
                shift 2
                ;;
            -w|--workers)
                WORKERS="$2"
                shift 2
                ;;
            -m|--max-len)
                MAX_LEN="$2"
                shift 2
                ;;
            -r|--rss-limit)
                RSS_LIMIT="$2"
                shift 2
                ;;
            --timeout)
                TIMEOUT="$2"
                shift 2
                ;;
            --dict)
                DICT_FILE="$2"
                shift 2
                ;;
            --corpus)
                WORK_CORPUS_DIR="$2"
                shift 2
                ;;
            --findings)
                FINDINGS_DIR="$2"
                shift 2
                ;;
            --debug)
                set -x
                shift
                ;;
            --continuous)
                CONTINUOUS=1
                shift
                ;;
            build|fuzz|minimize|verify|coverage|clean|reproduce|help)
                COMMAND="$1"
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                if [ "$COMMAND" = "reproduce" ] && [ -z "$CRASH_FILE" ]; then
                    CRASH_FILE="$1"
                else
                    error "Unknown argument: $1"
                    exit 1
                fi
                shift
                ;;
        esac
    done
}

# Main execution
main() {
    log "wiRedPanda Fuzzing Script v1.0"
    
    # Check prerequisites
    check_clang || exit 1
    
    case $COMMAND in
        build)
            build_fuzzer
            ;;
        fuzz)
            verify_setup && run_fuzzer
            ;;
        minimize)
            minimize_crashes
            ;;
        verify)
            verify_setup
            ;;
        coverage)
            generate_coverage
            ;;
        clean)
            clean_artifacts
            ;;
        reproduce)
            reproduce_crash "$CRASH_FILE"
            ;;
        help)
            show_help
            ;;
        *)
            error "Unknown command: $COMMAND"
            show_help
            exit 1
            ;;
    esac
}

# Parse arguments and run
parse_args "$@"
main