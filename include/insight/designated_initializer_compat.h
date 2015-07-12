/*
 * This file is part of Insight.
 *
 * Copyright © 2015 Franklin "Snaipe" Mathieu <http://snaipe.me>
 *
 * Insight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Insight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Insight.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef DESIGNATED_INITIALIZER_COMPAT_H_
# define DESIGNATED_INITIALIZER_COMPAT_H_

/*
 * This header file provides macro to emulate the designated initializer syntax
 * available in C, to make a compatibility bridge with the C++ interface.
 *
 * This relies on the fact that "Type symbol = { symbol.field, ... }
 * is a valid syntax in C++, so we use a macro to prepend the symbol name over
 * each ".field"
 *
 * The application macro goes from 2 to 65 because __VA_ARGS__ is prepended with
 * two unused parameters, for maximal compatibility (this avoids the expansion
 * of INSIGHT_ARG_LENGTH with no parameter and being forced to use the ## gnu macro extension.
 */

# include "vararg.h"

# define INSIGHT_ANNOTATION_APPLY_2(Macro, ...)
# define INSIGHT_ANNOTATION_APPLY_3(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_2(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_4(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_3(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_5(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_4(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_6(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_5(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_7(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_6(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_8(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_7(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_9(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_8(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_10(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_9(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_11(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_10(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_12(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_11(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_13(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_12(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_14(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_13(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_15(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_14(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_16(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_15(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_17(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_16(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_18(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_17(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_19(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_18(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_20(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_19(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_21(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_20(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_22(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_21(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_23(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_22(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_24(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_23(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_25(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_24(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_26(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_25(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_27(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_26(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_28(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_27(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_29(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_28(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_30(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_29(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_31(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_30(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_32(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_31(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_33(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_32(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_34(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_33(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_35(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_34(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_36(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_35(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_37(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_36(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_38(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_37(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_39(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_38(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_40(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_39(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_41(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_40(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_42(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_41(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_43(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_42(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_44(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_43(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_45(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_44(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_46(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_45(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_47(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_46(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_48(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_47(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_49(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_48(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_50(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_49(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_51(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_50(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_52(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_51(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_53(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_52(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_54(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_53(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_55(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_54(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_56(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_55(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_57(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_56(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_58(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_57(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_59(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_58(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_60(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_59(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_61(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_60(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_62(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_61(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_63(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_62(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_64(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_63(Macro, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_65(Macro, Prefix, Head, ...) Macro(Prefix, Head) INSIGHT_ANNOTATION_APPLY_64(Macro, __VA_ARGS__)

# define INSIGHT_ANNOTATION_APPLY__(Macro, Prefix, n, ...) INSIGHT_ANNOTATION_APPLY_##n(Macro, Prefix, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY_(Macro, n, Prefix, _, ...) INSIGHT_ANNOTATION_APPLY__(Macro, Prefix, n, __VA_ARGS__)
# define INSIGHT_ANNOTATION_APPLY(Macro, ...) INSIGHT_ANNOTATION_APPLY_(Macro, INSIGHT_ARG_LENGTH(__VA_ARGS__), __VA_ARGS__)

# define INSIGHT_ANNOTATION_ADD_PREFIX_ONCE(Prefix, Field, ...) Prefix Field,
# define INSIGHT_ANNOTATION_ADD_PREFIX(...) \
    INSIGHT_ANNOTATION_APPLY(INSIGHT_ANNOTATION_ADD_PREFIX_ONCE, __VA_ARGS__)

#endif /* !DESIGNATED_INITIALIZER_COMPAT_H_ */
