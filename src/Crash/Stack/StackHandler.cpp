#include "Crash/Stack/StackHandler.h"

#include "Crash/Modules/ModuleHandler.h"

namespace Crash
{
	namespace Stack
	{
		namespace detail
		{
			class NiNode
			{
			public:
				using value_type = RE::NiNode;

				static void filter(
					std::vector<std::pair<std::string, std::string>>& a_results,
					const void* a_ptr) noexcept
				{
					const auto texture = static_cast<const value_type*>(a_ptr);

					if (const auto name = GetName(texture); name) {
						a_results.emplace_back(
							"Name"sv,
							*name);
					}
				}

			private:
				[[nodiscard]] static auto GetName(const value_type* a_texture) noexcept
					-> std::optional<std::string_view>
				{
					__try {
						return a_texture->name;
					} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
						return std::nullopt;
					}
				}
			};

			class NiTexture
			{
			public:
				using value_type = RE::NiTexture;

				static void filter(
					std::vector<std::pair<std::string, std::string>>& a_results,
					const void* a_ptr) noexcept
				{
					const auto texture = static_cast<const value_type*>(a_ptr);

					if (const auto name = GetName(texture); name) {
						a_results.emplace_back(
							"Name"sv,
							*name);
					}
				}

			private:
				[[nodiscard]] static auto GetName(const value_type* a_texture) noexcept
					-> std::optional<std::string_view>
				{
					__try {
						return a_texture->name;
					} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
						return std::nullopt;
					}
				}
			};

			class TESForm
			{
			public:
				using value_type = RE::TESForm;

				static void filter(
					std::vector<std::pair<std::string, std::string>>& a_results,
					const void* a_ptr) noexcept
				{
					const auto form = static_cast<const value_type*>(a_ptr);

					if (const auto filename = GetFilename(form); filename) {
						a_results.emplace_back(
							"File"sv,
							*filename);
					}

					if (const auto formFlags = GetFormFlags(form); formFlags) {
						a_results.emplace_back(
							"Flags"sv,
							fmt::format(
								FMT_STRING("0x{:08X}"),
								*formFlags));
					}

					if (const auto formID = GetFormID(form); formID) {
						a_results.emplace_back(
							"Form ID"sv,
							fmt::format(
								FMT_STRING("0x{:08X}"),
								*formID));
					}
				}

			private:
				[[nodiscard]] static auto GetFilename(const value_type* a_form) noexcept
					-> std::optional<std::string_view>
				{
					__try {
						const auto files = a_form->sourceFiles.array;
						const auto file = files && !files->empty() ? files->back() : nullptr;
						if (file) {
							return file->GetFilename();
						} else {
							return std::nullopt;
						}
					} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
						return std::nullopt;
					}
				}

				[[nodiscard]] static auto GetFormFlags(const value_type* a_form) noexcept
					-> std::optional<std::uint32_t>
				{
					__try {
						return a_form->GetFormFlags();
					} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
						return std::nullopt;
					}
				}

				[[nodiscard]] static auto GetFormID(const value_type* a_form) noexcept
					-> std::optional<std::uint32_t>
				{
					__try {
						return a_form->GetFormID();
					} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
						return std::nullopt;
					}
				}
			};

			class TESFullName
			{
			public:
				using value_type = RE::TESFullName;

				static void filter(
					std::vector<std::pair<std::string, std::string>>& a_results,
					const void* a_ptr) noexcept
				{
					const auto form = static_cast<const value_type*>(a_ptr);

					if (const auto fullName = GetFullName(form); fullName) {
						a_results.emplace_back(
							"Full Name"sv,
							*fullName);
					}
				}

			private:
				[[nodiscard]] static auto GetFullName(const value_type* a_form) noexcept
					-> std::optional<std::string_view>
				{
					__try {
						return a_form->fullName;
					} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
						return std::nullopt;
					}
				}
			};

			class Integer
			{
			public:
				[[nodiscard]] std::string name() const noexcept { return "(size_t)"; }
			};

			class Pointer
			{
			public:
				[[nodiscard]] std::string name() const noexcept { return "(void*)"; }
			};

			class Polymorphic
			{
			public:
				Polymorphic(std::string_view a_mangled) noexcept :
					_mangled{ a_mangled }
				{
					assert(_mangled.size() > 1 && _mangled.data()[_mangled.size()] == '\0');
				}

				[[nodiscard]] std::string name() const noexcept
				{
					std::array<char, 0x1000> buf;
					const auto len =
						WinAPI::UnDecorateSymbolName(
							_mangled.data() + 1,
							buf.data(),
							static_cast<std::uint32_t>(buf.size()),
							(WinAPI::UNDNAME_NO_MS_KEYWORDS) |
								(WinAPI::UNDNAME_NO_FUNCTION_RETURNS) |
								(WinAPI::UNDNAME_NO_ALLOCATION_MODEL) |
								(WinAPI::UNDNAME_NO_ALLOCATION_LANGUAGE) |
								(WinAPI::UNDNAME_NO_THISTYPE) |
								(WinAPI::UNDNAME_NO_ACCESS_SPECIFIERS) |
								(WinAPI::UNDNAME_NO_THROW_SIGNATURES) |
								(WinAPI::UNDNAME_NO_RETURN_UDT_MODEL) |
								(WinAPI::UNDNAME_NAME_ONLY) |
								(WinAPI::UNDNAME_NO_ARGUMENTS) |
								static_cast<std::uint32_t>(0x8000));  // Disable enum/class/struct/union prefix

					if (len != 0) {
						return fmt::format(
							FMT_STRING("({}*)"),
							std::string_view{ buf.data(), len });
					} else {
						return "(ERROR)"s;
					}
				}

			private:
				std::string_view _mangled;
			};

			class F4Polymorphic
			{
			public:
				F4Polymorphic(
					std::string_view a_mangled,
					const RE::RTTI::CompleteObjectLocator* a_col,
					const void* a_ptr) noexcept :
					_poly{ a_mangled },
					_col{ a_col },
					_ptr{ a_ptr }
				{
					assert(_col != nullptr);
					assert(_ptr != nullptr);
				}

				[[nodiscard]] std::string name() const noexcept
				{
					auto result = _poly.name();
					std::vector<std::pair<std::string, std::string>> xInfo;

					const auto moduleBase = REL::Module::get().base();
					const auto hierarchy = _col->classDescriptor.get();
					const stl::span bases(
						reinterpret_cast<std::uint32_t*>(hierarchy->baseClassArray.offset() + moduleBase),
						hierarchy->numBaseClasses);
					for (const auto rva : bases) {
						const auto base = reinterpret_cast<RE::RTTI::BaseClassDescriptor*>(rva + moduleBase);
						const auto it = FILTERS.find(base->typeDescriptor->mangled_name());
						if (it != FILTERS.end()) {
							const auto root = stl::adjust_pointer<void>(_ptr, -static_cast<std::ptrdiff_t>(_col->offset));
							const auto target = stl::adjust_pointer<void>(root, static_cast<std::ptrdiff_t>(base->pmd.mDisp));
							it->second(xInfo, target);
						}
					}

					if (!xInfo.empty()) {
						std::sort(
							xInfo.begin(),
							xInfo.end(),
							[](auto&& a_lhs, auto&& a_rhs) {
								return a_lhs.first < a_rhs.first;
							});
						for (const auto& [key, value] : xInfo) {
							result += fmt::format(
								FMT_STRING("\n\t\t{}: {}"),
								key,
								value);
						}
					}

					return result;
				}

			private:
				static constexpr auto FILTERS = frozen::make_map({
					std::make_pair(".?AVNiNode@@"sv, NiNode::filter),
					std::make_pair(".?AVNiTexture@@"sv, NiTexture::filter),
					std::make_pair(".?AVTESForm@@"sv, TESForm::filter),
					std::make_pair("?AVTESFullName@@"sv, TESFullName::filter),
				});

				Polymorphic _poly;
				const RE::RTTI::CompleteObjectLocator* _col{ nullptr };
				const void* _ptr{ nullptr };
			};

			using analysis_result =
				std::variant<
					Integer,
					Pointer,
					Polymorphic,
					F4Polymorphic>;

			[[nodiscard]] const Modules::Module* get_module_for_pointer(
				void* a_ptr,
				stl::span<const module_pointer> a_modules) noexcept
			{
				const auto it = std::lower_bound(
					a_modules.rbegin(),
					a_modules.rend(),
					reinterpret_cast<std::uintptr_t>(a_ptr),
					[](auto&& a_lhs, auto&& a_rhs) noexcept {
						return a_lhs->address() >= a_rhs;
					});
				return it != a_modules.rend() && (*it)->in_range(a_ptr) ? it->get() : nullptr;
			}

			[[nodiscard]] auto analyze_pointer(
				void* a_ptr,
				stl::span<const module_pointer> a_modules) noexcept
				-> analysis_result
			{
				__try {
					const auto vtable = *reinterpret_cast<void**>(a_ptr);
					const auto mod = get_module_for_pointer(vtable, a_modules);
					if (!mod || !mod->in_rdata_range(vtable)) {
						return Pointer{};
					}

					const auto col =
						*reinterpret_cast<RE::RTTI::CompleteObjectLocator**>(
							reinterpret_cast<std::size_t*>(vtable) - 1);
					if (mod != get_module_for_pointer(col, a_modules) || !mod->in_rdata_range(col)) {
						return Pointer{};
					}

					const auto typeDesc =
						reinterpret_cast<RE::RTTI::TypeDescriptor*>(
							mod->address() + col->typeDescriptor.offset());
					if (mod != get_module_for_pointer(typeDesc, a_modules) || !mod->in_data_range(typeDesc)) {
						return Pointer{};
					}

					if (*reinterpret_cast<const void**>(typeDesc) != mod->type_info()) {
						return Pointer{};
					}

					if (_stricmp(mod->name().data(), "Fallout4.exe") == 0) {
						return F4Polymorphic{ typeDesc->mangled_name(), col, a_ptr };
					} else {
						return Polymorphic{ typeDesc->mangled_name() };
					}
				} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
					return Pointer{};
				}
			}

			[[nodiscard]] auto analyze_integer(
				std::size_t a_value,
				stl::span<const module_pointer> a_modules) noexcept
				-> analysis_result
			{
				__try {
					*reinterpret_cast<const volatile std::byte*>(a_value);
					return analyze_pointer(reinterpret_cast<void*>(a_value), a_modules);
				} __except (WinAPI::EXCEPTION_EXECUTE_HANDLER) {
					return Integer{};
				}
			}
		}

		std::vector<std::string> analyze_stack(
			stl::span<const std::size_t> a_stack,
			stl::span<const module_pointer> a_modules) noexcept
		{
			std::vector<std::string> results;
			results.resize(a_stack.size());
			std::for_each(
				std::execution::parallel_unsequenced_policy{},
				a_stack.begin(),
				a_stack.end(),
				[&](auto& a_val) {
					const auto result = detail::analyze_integer(a_val, a_modules);
					const auto pos = std::addressof(a_val) - a_stack.data();
					results[pos] =
						std::visit(
							[](auto&& a_val) noexcept { return a_val.name(); },
							result);
				});
			return results;
		}
	}
}