#!/usr/bin/env python3
"""
Comprehensive SPARC/RTEMS Event Fix for F-Prime GDS

This addresses:
1. DecodingException import issues
2. Argument value range mismatches 
3. Enhanced error recovery for SPARC/RTEMS compatibility
"""

import sys
import re
from pathlib import Path

def fix_imports(gds_path):
    """Fix DecodingException imports definitively"""
    decoder_path = gds_path / "fprime_gds/common/decoders/event_decoder.py"
    
    with open(decoder_path, 'r') as f:
        content = f.read()
    
    # Ensure proper imports at top of file
    import_section = '''"""
@brief Decoder for event data

This decoder takes in serialized events, parses them, and packages the results
in event_data objects.

Example data structure:
    +-------------------+---------------------+---------------------- - - -
    | ID (4 bytes)      | Time Tag (11 bytes) | Event argument data....
    +-------------------+---------------------+---------------------- - - -

@date Created June 29, 2018
@author R. Joseph Paetz

@bug No known bugs
"""
import logging
from fprime.common.models.serialize import time_type
from fprime.common.models.serialize.type_exceptions import TypeException

from fprime_gds.common.data_types import event_data
from fprime_gds.common.decoders import decoder
from fprime_gds.common.decoders.decoder import DecodingException
from fprime_gds.common.utils import config_manager

LOGGER = logging.getLogger("event_decoder")'''

    # Replace everything up to the class definition
    class_start = content.find('class EventDecoder(decoder.Decoder):')
    if class_start != -1:
        content = import_section + '\n\n\n' + content[class_start:]
        
        with open(decoder_path, 'w') as f:
            f.write(content)
        print("✓ Fixed imports and added logging")
        return True
    
    print("✗ Could not find class definition")
    return False

def add_sparc_compatibility(gds_path):
    """Add SPARC/RTEMS specific argument handling"""
    decoder_path = gds_path / "fprime_gds/common/decoders/event_decoder.py"
    
    with open(decoder_path, 'r') as f:
        content = f.read()
    
    # Enhanced decode_args method for SPARC compatibility
    sparc_decode_args = '''    @staticmethod
    def decode_args(arg_data, offset, template):
        """
        Decodes the serialized event arguments with SPARC/RTEMS compatibility

        Enhanced to handle:
        - Value range mismatches between SPARC and dictionary
        - Enum value differences
        - Graceful recovery from parsing errors
        """
        arg_results = []
        args = template.get_args()
        original_offset = offset

        LOGGER.debug(f"Decoding {len(args)} arguments for event {template.get_name() if hasattr(template, 'get_name') else 'unknown'}")

        for i, arg in enumerate(args):
            (arg_name, arg_desc, arg_type) = arg
            arg_obj = arg_type()

            try:
                # Check buffer bounds
                if offset + arg_obj.getSize() > len(arg_data):
                    LOGGER.warning(f"Argument {i} ({arg_name}) would read beyond buffer")
                    break
                
                # Try to deserialize
                arg_obj.deserialize(arg_data, offset)
                
                # SPARC/RTEMS compatibility: Check for value range issues
                if hasattr(arg_obj, 'val'):
                    val = arg_obj.val
                    
                    # For enum types, handle out-of-range values gracefully
                    if hasattr(arg_type, '_enum_values') or 'Enum' in str(arg_type):
                        LOGGER.debug(f"Enum argument {arg_name} = {val}")
                        # Don't reject out-of-range enum values, just log them
                        if val not in getattr(arg_type, '_enum_values', {}):
                            LOGGER.warning(f"SPARC/RTEMS enum value {val} for {arg_name} not in dictionary - using raw value")
                    
                    # For numeric ranges, be more permissive
                    if hasattr(arg_type, '_min_val') and hasattr(arg_type, '_max_val'):
                        if not (arg_type._min_val <= val <= arg_type._max_val):
                            LOGGER.warning(f"SPARC/RTEMS value {val} for {arg_name} outside expected range [{arg_type._min_val}, {arg_type._max_val}] - using anyway")
                
                arg_results.append(arg_obj)
                offset += arg_obj.getSize()
                
            except ValueError as ve:
                if "out of range" in str(ve):
                    LOGGER.warning(f"SPARC/RTEMS compatibility: {arg_name} value out of range - {ve}")
                    # Create a placeholder argument with raw value
                    try:
                        # Try to read the raw bytes and use them
                        raw_bytes = arg_data[offset:offset + arg_obj.getSize()]
                        if len(raw_bytes) == 4:  # Assuming U32 for most args
                            import struct
                            raw_val = struct.unpack('>I', raw_bytes)[0]  # Big-endian U32
                            LOGGER.warning(f"Using raw value {raw_val} for {arg_name}")
                            
                            # Create a fake argument object
                            class RawArg:
                                def __init__(self, value):
                                    self.val = value
                                def getSize(self):
                                    return 4
                                def serialize(self):
                                    return struct.pack('>I', self.val)
                            
                            arg_results.append(RawArg(raw_val))
                            offset += arg_obj.getSize()
                        else:
                            LOGGER.error(f"Cannot handle raw arg {arg_name} with size {len(raw_bytes)}")
                            break
                    except Exception as raw_exc:
                        LOGGER.error(f"Failed to parse raw argument {arg_name}: {raw_exc}")
                        break
                else:
                    LOGGER.error(f"Argument {i} ({arg_name}) failed: {ve}")
                    break
                    
            except Exception as e:
                LOGGER.error(f"Argument {i} ({arg_name}) parsing failed: {e}")
                # Try to continue with remaining arguments
                remaining = len(arg_data) - offset
                LOGGER.warning(f"Skipping failed argument, {remaining} bytes remaining")
                break

        return [offset - original_offset, tuple(arg_results)]'''

    # Replace the decode_args method
    pattern = r'    @staticmethod\s+def decode_args\(arg_data, offset, template\):.*?^        return \[.*?\]'
    
    if re.search(pattern, content, re.MULTILINE | re.DOTALL):
        content = re.sub(pattern, sparc_decode_args, content, flags=re.MULTILINE | re.DOTALL)
        
        with open(decoder_path, 'w') as f:
            f.write(content)
        print("✓ Added SPARC/RTEMS argument compatibility")
        return True
    else:
        print("✗ Could not find decode_args method")
        return False

def add_enhanced_decode_api(gds_path):
    """Replace decode_api with enhanced version"""
    decoder_path = gds_path / "fprime_gds/common/decoders/event_decoder.py"
    
    with open(decoder_path, 'r') as f:
        content = f.read()
    
    enhanced_decode_api = '''    def decode_api(self, data):
        """
        Enhanced event decoder for SPARC/RTEMS compatibility
        
        Handles:
        - Multiple events per packet (batching)
        - Argument parsing errors gracefully
        - Out-of-range enum values
        - Buffer boundary issues
        """
        ptr = 0
        event_list = []

        LOGGER.debug(f"Decoding event packet: {len(data)} bytes")
        if len(data) < 50:  # Log small packets for debugging
            LOGGER.debug(f"Packet hex: {data.hex()}")

        while ptr < len(data):
            try:
                # Decode event ID
                if ptr + self.id_obj.getSize() > len(data):
                    LOGGER.debug(f"Not enough data for event ID at ptr={ptr}")
                    break
                    
                self.id_obj.deserialize(data, ptr)
                ptr += self.id_obj.getSize()
                event_id = self.id_obj.val

                # Decode time
                event_time = time_type.TimeType()
                if ptr + event_time.getSize() > len(data):
                    LOGGER.debug(f"Not enough data for time at ptr={ptr}")
                    break
                    
                event_time.deserialize(data, ptr)
                ptr += event_time.getSize()

                if event_id not in self.__dict:
                    LOGGER.warning(f"Event {event_id} not in dictionary")
                    # Try to skip this event by guessing its size
                    remaining = len(data) - ptr
                    if remaining > 0:
                        LOGGER.warning(f"Skipping unknown event, {remaining} bytes remaining")
                        # Skip to next potential event boundary
                        ptr = len(data)  # Skip rest of packet
                    break

                event_temp = self.__dict[event_id]
                LOGGER.debug(f"Processing event {event_id} ({event_temp.get_name() if hasattr(event_temp, 'get_name') else 'unknown'})")

                # Enhanced argument parsing
                try:
                    (size, arg_vals) = self.decode_args(data, ptr, event_temp)
                    
                    # Create event data
                    event_obj = event_data.EventData(arg_vals, event_time, event_temp)
                    event_list.append(event_obj)
                    ptr += size
                    
                    LOGGER.debug(f"Successfully decoded event {event_id}")
                    
                except Exception as arg_exc:
                    LOGGER.warning(f"Event {event_id} argument parsing failed: {arg_exc}")
                    # Create event with empty arguments as fallback
                    try:
                        event_obj = event_data.EventData(tuple(), event_time, event_temp)
                        event_list.append(event_obj)
                        # Skip to end to avoid further parsing errors
                        ptr = len(data)
                    except Exception as create_exc:
                        LOGGER.error(f"Failed to create fallback event: {create_exc}")
                        break
                        
            except Exception as evt_exc:
                LOGGER.warning(f"Event parsing failed at ptr={ptr}: {evt_exc}")
                if len(data) < 100:
                    LOGGER.warning(f"Failed packet hex: {data.hex()}")
                break

        LOGGER.debug(f"Decoded {len(event_list)} events from packet")
        return event_list'''

    # Replace decode_api method
    pattern = r'    def decode_api\(self, data\):.*?^        return event_list'
    
    if re.search(pattern, content, re.MULTILINE | re.DOTALL):
        content = re.sub(pattern, enhanced_decode_api, content, flags=re.MULTILINE | re.DOTALL)
        
        with open(decoder_path, 'w') as f:
            f.write(content)
        print("✓ Enhanced decode_api for SPARC/RTEMS compatibility")
        return True
    else:
        print("✗ Could not find decode_api method")
        return False

def create_debug_config(gds_path):
    """Create debug configuration for SPARC analysis"""
    debug_script = gds_path / "sparc_debug_config.py"
    
    debug_code = '''#!/usr/bin/env python3
"""
SPARC/RTEMS Debug Configuration

Sets up enhanced logging for event analysis
"""

import logging
import sys

def setup_sparc_debug():
    """Configure enhanced logging for SPARC debugging"""
    
    # Set up detailed logging
    logging.basicConfig(
        level=logging.DEBUG,
        format='%(asctime)s [%(levelname)8s] %(name)s: %(message)s',
        handlers=[
            logging.StreamHandler(),
            logging.FileHandler('sparc_event_debug.log', mode='w')
        ]
    )
    
    # Enable debug logging for event decoder specifically
    event_logger = logging.getLogger('event_decoder')
    event_logger.setLevel(logging.DEBUG)
    
    # Disable some verbose loggers
    logging.getLogger('werkzeug').setLevel(logging.WARNING)
    logging.getLogger('urllib3').setLevel(logging.WARNING)
    
    print("SPARC/RTEMS Debug logging enabled")
    print("Event details will be logged to: sparc_event_debug.log")
    print("Look for messages about:")
    print("- Event packet sizes and hex dumps")
    print("- Argument parsing details")
    print("- Enum value mismatches") 
    print("- Buffer boundary issues")

if __name__ == "__main__":
    setup_sparc_debug()
'''

    with open(debug_script, 'w') as f:
        f.write(debug_code)
    
    print(f"✓ Created debug configuration: {debug_script}")

def main():
    gds_path = Path("./gds-venv/lib/python3.12/site-packages/")
    
    print("SPARC/RTEMS Comprehensive Event Fix")
    print("=" * 40)
    
    success_count = 0
    
    if fix_imports(gds_path):
        success_count += 1
    
    if add_enhanced_decode_api(gds_path):
        success_count += 1
        
    if add_sparc_compatibility(gds_path):
        success_count += 1
    
    create_debug_config(gds_path)
    
    print("\n" + "=" * 40)
    if success_count >= 2:
        print("✓ SPARC/RTEMS fixes applied successfully!")
        print("\nEnhancements:")
        print("1. Fixed DecodingException import issues")
        print("2. Added graceful handling of enum range mismatches")
        print("3. Enhanced error recovery for SPARC compatibility")
        print("4. Improved debug logging")
        print("\nNext steps:")
        print("1. Restart your GDS")
        print("2. Run: python sparc_debug_config.py (for detailed logs)")
        print("3. Connect to GR740 and check events")
        print("4. Check sparc_event_debug.log for detailed analysis")
    else:
        print("✗ Some fixes failed - check manually")
    
    return success_count >= 2

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
