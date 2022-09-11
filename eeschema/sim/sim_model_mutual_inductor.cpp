/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2022 Mikolaj Wielgus
 * Copyright (C) 2022 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <sim/sim_model_mutual_inductor.h>


SIM_MODEL_MUTUAL_INDUCTOR::SIM_MODEL_MUTUAL_INDUCTOR() : SIM_MODEL( SIM_MODEL::TYPE::L_MUTUAL )
{
    static std::vector<PARAM::INFO> paramInfos = makeParamInfos();

    for( const PARAM::INFO& paramInfo : paramInfos )
        AddParam( paramInfo );
}


wxString SIM_MODEL_MUTUAL_INDUCTOR::GenerateSpiceItemParamValuePair( const PARAM& aParam,
                                                                     bool& aIsFirst ) const
{
    wxString result;

    result << aParam.value->ToSimpleString() << " ";
    return result;
}


const std::vector<SIM_MODEL::PARAM::INFO> SIM_MODEL_MUTUAL_INDUCTOR::makeParamInfos()
{
    std::vector<PARAM::INFO> paramInfos;
    PARAM::INFO paramInfo;

    paramInfo.name = "l1";
    paramInfo.type = SIM_VALUE::TYPE_STRING;
    paramInfo.unit = "";
    paramInfo.category = PARAM::CATEGORY::PRINCIPAL;
    paramInfo.defaultValue = "";
    paramInfo.description = "Inductor 1";
    paramInfo.isSpiceInstanceParam = true;
    paramInfos.push_back( paramInfo );

    paramInfo.name = "l2";
    paramInfo.type = SIM_VALUE::TYPE_STRING;
    paramInfo.unit = "";
    paramInfo.category = PARAM::CATEGORY::PRINCIPAL;
    paramInfo.defaultValue = "";
    paramInfo.description = "Inductor 2";
    paramInfo.isSpiceInstanceParam = true;
    paramInfos.push_back( paramInfo );

    paramInfo.name = "k";
    paramInfo.type = SIM_VALUE::TYPE_FLOAT;
    paramInfo.unit = "";
    paramInfo.category = PARAM::CATEGORY::PRINCIPAL;
    paramInfo.defaultValue = "";
    paramInfo.description = "Coupling coefficient";
    paramInfo.isSpiceInstanceParam = true;
    paramInfos.push_back( paramInfo );

    return paramInfos;
}
