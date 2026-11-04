use std::ffi::CStr;
use std::os::raw::{c_char, c_int};
use std::path::Path;

#[unsafe(no_mangle)]
pub extern "C" fn get_fuzzy_score_rs(query_ptr: *const c_char, path_ptr: *const c_char) -> c_int {
    if query_ptr.is_null() || path_ptr.is_null() {
        return 0;
    }

    let query = unsafe { CStr::from_ptr(query_ptr).to_string_lossy().to_lowercase() };
    let path = unsafe { CStr::from_ptr(path_ptr).to_string_lossy() };

    if query.is_empty() {
        return 1;
    }

    let basename = Path::new(path.as_ref())
        .file_name()
        .unwrap_or_default()
        .to_string_lossy()
        .to_lowercase();

    // Tier 3: 精確命中 (Exact Match)
    if basename == query {
        return 3;
    }

    // Tier 2: 包含命中/前綴命中 (Substring Match)
    if basename.contains(&query) {
        return 2;
    }

    // Tier 1: 模糊跳躍命中 (Fuzzy Match)
    let mut q_chars = query.chars().peekable();
    for c in basename.chars() {
        if let Some(&q_c) = q_chars.peek() {
            if c == q_c {
                q_chars.next();
            }
        }
        if q_chars.peek().is_none() {
            return 1;
        }
    }

    // Tier 0: 毫無關係
    0
}
