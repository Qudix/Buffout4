#pragma once

namespace boost::stacktrace
{
	class frame;
}

namespace Crash
{
	namespace Modules
	{
		namespace detail
		{
			class Factory;
		}

		class Module
		{
		public:
			virtual ~Module() noexcept = default;

			[[nodiscard]] std::uintptr_t address() const noexcept { return reinterpret_cast<std::uintptr_t>(_image.data()); }

			[[nodiscard]] std::string frame_info(const boost::stacktrace::frame& a_frame) const noexcept;

			[[nodiscard]] bool in_range(const void* a_ptr) const noexcept
			{
				const auto ptr = reinterpret_cast<const std::byte*>(a_ptr);
				return _image.data() <= ptr && ptr < _image.data() + _image.size();
			}

			[[nodiscard]] bool in_data_range(const void* a_ptr) const noexcept
			{
				const auto ptr = reinterpret_cast<const std::byte*>(a_ptr);
				return _data.data() <= ptr && ptr < _data.data() + _data.size();
			}

			[[nodiscard]] bool in_rdata_range(const void* a_ptr) const noexcept
			{
				const auto ptr = reinterpret_cast<const std::byte*>(a_ptr);
				return _rdata.data() <= ptr && ptr < _rdata.data() + _rdata.size();
			}

			[[nodiscard]] std::string_view name() const noexcept { return _name; }

			[[nodiscard]] const RE::msvc::type_info* type_info() const noexcept { return _typeInfo; }

		protected:
			friend class detail::Factory;

			Module(std::string a_name, stl::span<const std::byte> a_image) noexcept;

			[[nodiscard]] virtual std::string get_frame_info(const boost::stacktrace::frame& a_frame) const noexcept;

		private:
			std::string _name;
			stl::span<const std::byte> _image;
			stl::span<const std::byte> _data;
			stl::span<const std::byte> _rdata;
			const RE::msvc::type_info* _typeInfo{ nullptr };
		};

		[[nodiscard]] auto get_loaded_modules() noexcept
			-> std::vector<std::unique_ptr<Module>>;
	}

	using module_pointer = std::unique_ptr<Modules::Module>;
}