import importlib.util
import sys
import os

def load_and_run_module(module_path):
    """Loads and executes a Python module from a given path."""

    module_name = os.path.basename(module_path).replace(".py", "")  # Extract module name

    spec = importlib.util.spec_from_file_location(module_name, module_path)
    if spec is None:
        raise ImportError(f"Could not find module {module_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module  # Important: Add module to sys.modules
    try:
        spec.loader.exec_module(module)
        return module #return module to call functions from it later
    except FileNotFoundError:
        print(f"File not found: {module_path}")
        return None
    except Exception as e:
        print(f"An error occurred: {e}")
        return None
    

# Example usage:
main_dir = os.path.dirname(__file__) #get the directory of the current file
#module_path = os.path.join(main_dir, "my_module.py")  # Path to the module you want to load
path = "/home/schoch/dev/now_pendant/linuxcnc_comp/now_pendant.py"
loaded_module = load_and_run_module(path)

#if loaded_module:
#    loaded_module.my_function() #call a function from imported module
#
#    print(loaded_module.my_variable) #print a variable from imported module
