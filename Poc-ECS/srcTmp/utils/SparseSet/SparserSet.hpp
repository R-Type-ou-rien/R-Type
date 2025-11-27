#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

#ifndef SPARSE_HPP

    using IdType = std::uint32_t;

    class ISparseSet {
    public:
        virtual ~ISparseSet() = default;
        virtual void remove(int entityId) = 0;
        virtual bool has(int entityId) const = 0;
    };

    template<typename data_type>
    class SparseSet {
        private:

            /** 
                A vector containing the index of the data in the dense vector at the id position.
                The index equal -1 if their is no data
            */
            std::vector<int> _sparse;

            /** A vector containing the data stored contigously */
            std::vector<data_type> _dense;

            /**
                A vector containing id at their data index positions.
                The id are stored contigously
            */
            std::vector<IdType> _reverse_dense;

            /** Index for the _dense and _reverse_dense vectors */
            std::size_t _index = 0;

        public:
            /**
                A function to add a new data to the given id
                @param IdType id
            */
            void addID(IdType id, const data_type& data)
            {
                if (id >= _sparse.size()) {
                    _sparse.resize(id + 1, -1);
                }
                _sparse[id] = _index;
                _dense.push_back(data);
                _reverse_dense.push_back(id);
                _index++;
                return;
            }

            /**
                A function to remove a data from the given id
                @param IdType id
            */
            void removeId(IdType id)
            {
                if (!has(id)) {
                    std::cerr << "Error: removeId: " << id 
                    << " does not have any components from this type." << std::endl;
                    return;
                }
                std::size_t indexToRemove = _sparse[id];
                std::size_t lastIndex = _index - 1;
                data_type lastData = _dense[lastIndex];

                _sparse[id] = -1;
                _sparse[lastIndex] = indexToRemove;
                _dense[indexToRemove] = _dense[lastData];
                _reverse_dense[indexToRemove] = lastData;
                _dense.pop_back();
                _reverse_dense.pop_back();
                _index--;
                std::cout << "Component from entity " << id << " successfully removed." << std::endl;
                return;
            }

            /**
                A function to check if an id has a data
            */
            bool has(IdType id) const
            {
                if (_sparse[id] > -1 && id < _sparse.size())
                    return true;
                return false;
            }

            /**
                A function to get the data of a given id
                @param IdType id
                @return The function returns reference to the corresponding data
            */
            data_type& getDataFromId(IdType id) const
            {
                return _dense[_sparse[id]];
            }

            /**
                A function to get the id of a given data
                @param data_type data
                @return The function returns the corresponding id 
            */
            IdType getIdFromData(data_type data) const
            {
                return _reverse_dense[data];
            }

            /**
                A function to get the data's list stored
                @return The function returns the dense vector
            */
            std::vector<data_type> getDataList() const
            {
                return _dense;
            }

            /**
                A function to get the id's list stored
                @return The function returns the _reverse_dense vector
            */
            std::vector<IdType> getIdList() const
            {
                return _reverse_dense;
            }
    };

#endif