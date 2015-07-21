#ifndef GENESIS_BOOST_PYTHON_EXPORT_COMMON_H_
#define GENESIS_BOOST_PYTHON_EXPORT_COMMON_H_

/**
 * @brief Definition of the genesis Python module.
 *
 * @file
 * @ingroup python
 */

#include <boost/python.hpp>

#include <vector>

// =================================================================================================
//     Inclusion of genesis
// =================================================================================================

// Dummy for the namespace.
namespace genesis {}

// This header is compiled into the python module, but not used in the library itself. Thus, we can
// include the whole namespace of genesis for ease of use here, without risking name conflicts.
using namespace genesis;

// =================================================================================================
//     Forward Declarations
// =================================================================================================

const char* get_docstring (const std::string& signature);

// =================================================================================================
//     Export Macros
// =================================================================================================

/**
 * @brief
 *
 * The class registration is performed by calling PythonExport::registerClassInitializer. In order
 * to call this when the program is started, we initiate a static object of type
 * RegisterPythonExportClass<classname> with name PythonExporterInstance##Classname; the constructor
 * calls (with an intermediate step) the function PythonExportWrapper<T>(). The body of the
 * PYTHON_EXPORT_CLASS(classname) { .. } macro is the body of PythonExportWrapper<classname>.
 */
#define PYTHON_EXPORT_CLASS(Classname) \
    namespace { RegisterPythonExportClass<Classname> PythonExporterInstance##Classname; } \
    template<> inline void PythonExportWrapper<Classname>()

/**
* @brief Call this macro inside the initialization function to tell the system that
* another class must be initialized previously (particularly, a base class)
*
*/
#define PYTHON_REQUIRES_CLASS(Classname) \
    RegisterPythonExportClass<Classname>::PythonExportWrapperDelegator();

/**
 * @brief Template for export definition function.
 *
 * This wrapper function template is instanciated for each class and contains the python class
 * definition as its body. Thus, we do not provide an implementation here - this will be done in the
 * respective instanciations for each class.
 */
template<typename T> void PythonExportWrapper();

// =================================================================================================
//     Python Export Handler
// =================================================================================================

/**
 * @brief
 */
class PythonExportHandler
{
public:
    /**
     * @brief Return single instance of this class.
     */
    static PythonExportHandler& instance() {
        // Meyers-Singleton
        static PythonExportHandler instance;
        return instance;
    }

    /**
     * @brief Register a Python initialization function.
     *
     * Functions registered first are called first.
     * Note that therefore, base classes must be registered before
     * derived classes, usually using the PYTHON_REQUIRES_CLASS macro.
     *
     * This function is called by the PYTHON_EXPORT_CLASS macro expansion.
     */
    void register_class_initializer(void (*func)()) {
        initializers.push_back(func);
    }

    /**
     * @brief Main Python initialization function.
     *
     * Must only be called after all initializers have been registered.
     */
    void init_python() {
        for (auto it = initializers.begin(); it != initializers.end(); ++it) {
            (*it)();
        }
    }

private:
    /**
     * @brief Private constructor, does nothing.
     */
    PythonExportHandler() {}

    /**
     * @brief List of initializer functions.
     */
    std::vector<void(*)()> initializers;
};

/**
 * @brief Helper class which is instantiated in order to register a Python export function.
 *
 * Do not use this class directly, it is used by the PYTHON_... macros.
 *
 * Rules (for experts only):
 * Call RegisterPythonExportClass<Classname>() to register class Classname for export.
 * The export specification is given in the global function PythonExportWrapper<Setname>.
 * That function is free to also export subclasses etc.
 */
template<typename T>
class RegisterPythonExportClass {
public:
    RegisterPythonExportClass() {
        PythonExportHandler::instance().register_class_initializer(
            &RegisterPythonExportClass<T>::PythonExportWrapperDelegator
        );
    }

    /**
     * @brief This tells the system that another class must be initialized previously
     * (particularly, a base class).
     */
    static void PythonExportWrapperDelegator() {
        static bool hasBeenCalled = false;
        if (!hasBeenCalled) {
            PythonExportWrapper<T>();
            hasBeenCalled = true;
        }
    }
};

#endif // include guard
