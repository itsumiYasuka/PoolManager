#pragma once
#include <cstdint>
#include <algorithm>

namespace rage
{
	class fwBasePool
	{
	protected:
		int8_t* m_unk; //0x0000
		int8_t* m_data;  //0x0008
		int8_t* m_flags; //0x0010
		int32_t m_size; //0x0018
		int32_t m_datasize; //0x001C
		int32_t m_firstList; //0x0020
		int32_t m_lastAlloc; //0x0024
		int32_t m_bitCount; //0x0028

	private:
		struct VirtualDtorBase
		{
			virtual ~VirtualDtorBase() = 0;
		};

	public:
		template<typename T>
		T* GetAt(int index) const
		{
			if (m_flags[index] < 0)
			{
				return nullptr;
			}

			return reinterpret_cast<T*>(m_data + (index * m_datasize));
		}

		template<typename T>
		T* GetAtHandle(int handle) const
		{
			int index = handle >> 8;
			int8_t flag = handle & 0xFF;

			if (m_flags[index] < 0 || m_flags[index] != flag)
			{
				return nullptr;
			}

			return reinterpret_cast<T*>(m_data + (index * m_datasize));
		}

		size_t GetCountDirect()
		{
			// R* code does << 2 >> 2, but that makes no sense
			return m_bitCount & 0x3FFFFFFF;
		}

		size_t GetCount()
		{
			size_t count = std::count_if(m_flags, m_flags + m_size, [](int8_t flag)
				{
					return (flag >= 0);
				});

			return count;
		}

		size_t GetSize()
		{
			return m_size;
		}

		size_t GetEntrySize()
		{
			return m_datasize;
		}

		void Clear()
		{
			for (int i = 0; i < m_size; i++)
			{
				if (m_flags[i] >= 0)
				{
					delete GetAt<VirtualDtorBase>(i);
				}
			}
		}
	};
}