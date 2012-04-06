/*
	Copyright (c) 2012, NuoDB, Inc.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		 * Redistributions of source code must retain the above copyright
			notice, this list of conditions and the following disclaimer.
		 * Redistributions in binary form must reproduce the above copyright
			notice, this list of conditions and the following disclaimer in the
			documentation and/or other materials provided with the distribution.
		 * Neither the name of NuoDB, Inc. nor the names of its contributors may
			be used to endorse or promote products derived from this software
			without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
	OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdafx.h"

#include "error.h"

#pragma region NuoDbError
#pragma region Construction / Destruction
NuoDb::NuoDbError::NuoDbError(System::String^ server, System::String^ message, int lineNumber, int number, Byte^ state) :
	m_source(".Net NuoDb Data Provider"),
	m_server(server),
	m_message(message),
	m_lineNumber(lineNumber),
	m_number(number),
	m_state(state)
{
}
#pragma endregion
#pragma endregion

#pragma region NuoDbErrorCollection
#pragma region Construction / Destruction
NuoDb::NuoDbErrorCollection::NuoDbErrorCollection()
{
	m_errors = gcnew List<NuoDbError^>();
}
#pragma endregion

#pragma region Properties
NuoDb::NuoDbError^ NuoDb::NuoDbErrorCollection::default::get(int index)
{
	if (index < 0 || index >= m_errors->Count)
		throw gcnew IndexOutOfRangeException("index");

	return m_errors[index];
}
#pragma endregion

#pragma region Methods
void NuoDb::NuoDbErrorCollection::Add(NuoDb::NuoDbError^ e)
{
	if (nullptr == e)
		throw gcnew ArgumentNullException("e");

	m_errors->Add(e);
}

void NuoDb::NuoDbErrorCollection::CopyTo(array<NuoDb::NuoDbError^>^ a, int index)
{
	if (nullptr == a)
		throw gcnew ArgumentNullException("a");

	if (index < 0 || index >= a->Length)
		throw gcnew IndexOutOfRangeException("index");

	m_errors->CopyTo(a, index);
}
#pragma endregion

#pragma region ICollection Implementation
void NuoDb::NuoDbErrorCollection::CopyTo(Array^ a, int index)
{
	if (nullptr == a)
		throw gcnew ArgumentNullException("a");

	if (index < 0 || index >= a->Length)
		throw gcnew IndexOutOfRangeException("index");

	Array::Copy((Array^)m_errors, a, index);
}
#pragma endregion
#pragma endregion
