use neon::prelude::*;
use lofty::{Accessor, AudioFile, Probe, FileProperties};
use std::{path::Path, ffi::OsStr, panic};
use walkdir::WalkDir;
extern crate neon;
extern crate base64;


#[neon::main]
fn main(mut cx: ModuleContext) -> NeonResult<()> {
    panic::set_hook(Box::new(|_info| {
      // do nothing
  }));
    cx.export_function("parseFile", parse_file)?;
    cx.export_function("recursiveFolderSearch", recursive_folder_search)?;
    Ok(())
}

pub fn file_extension_parser(filename: &str) -> Option<&str> {    
  Path::new(filename).extension().and_then(OsStr::to_str)
}

// Stolen from official Neon docs.
pub fn vec_to_array<'a, C: Context<'a>>(vec: &Vec<String>, cx: &mut C) -> JsResult<'a, JsArray> {
  let a = JsArray::new(cx, vec.len() as u32);

  for (i, s) in vec.iter().enumerate() {
      let v = cx.string(s);
      a.set(cx, i as u32, v)?;
  }

  Ok(a)
}

pub fn recursive_folder_search(mut cx: FunctionContext) -> JsResult<JsObject> {
    let arg: Handle<JsString> = cx.argument(0)?;
    
    let mut parse_file_vec: Vec<String> = Vec::new();
    let mut music_metadata_vec: Vec<String> = Vec::new();

    for file in WalkDir::new(arg.value(&mut cx)).follow_links(true).into_iter().filter_map(|e| e.ok()) {
      let file_name_ascii_lowercase = file.file_name().to_ascii_lowercase();
      let file_name_str = file_extension_parser(file_name_ascii_lowercase.to_str().unwrap());
      match file_name_str.unwrap()  {
        "mp3" | "flac" | "wav" | "opus" => parse_file_vec.push(file_name_ascii_lowercase.to_str().unwrap().to_owned()),
        "aac" | "m4a" | "ogg" | "webm" => music_metadata_vec.push(file_name_ascii_lowercase.to_str().unwrap().to_owned()),
        _ => continue,
      };
    }
    
    let result_obj: Handle<JsObject> = cx.empty_object();
    let parse_file_array = vec_to_array(&parse_file_vec, &mut cx).unwrap();
    let mm_file_array = vec_to_array(&music_metadata_vec, &mut cx).unwrap();

    result_obj.set(&mut cx, "parseFile", parse_file_array)?;
    result_obj.set(&mut cx, "musicMetadata", mm_file_array)?;
    
    Ok(result_obj)
}

pub fn parse_file(mut cx: FunctionContext) -> JsResult<JsObject> {

    let arg: Handle<JsString> = cx.argument(0)?; 
    let path_str: String = arg.value(&mut cx);
    let path: &Path = Path::new(&path_str);

    let tagged_file = Probe::open(path)
		.expect("ERROR: Bad path provided!")
		.read(true)
		.expect("ERROR: Failed to read file!");

    let tag = match tagged_file.primary_tag() {
		Some(primary_tag) => primary_tag,
		// If the "primary" tag doesn't exist, we just grab the
		// first tag we can find. Realistically, a tag reader would likely
		// iterate through the tags to find a suitable one.
		None => tagged_file.first_tag().expect("ERROR: No tags found!"),
	  };

    let properties: &FileProperties = tagged_file.properties();
    let duration: u128 = properties.duration().as_millis();
    //let picture: &[Picture] = tag.pictures();
    let file_ext = path.extension().and_then(OsStr::to_str);
    let metadata_obj: Handle<JsObject> = cx.empty_object();

    let title: Handle<JsString> = cx.string(tag.title().unwrap_or("0"));
    let artist: Handle<JsString> = cx.string(tag.artist().unwrap_or("0"));
    let album: Handle<JsString> = cx.string(tag.album().unwrap_or("0"));
    let genre: Handle<JsString> = cx.string(tag.genre().unwrap_or("0"));
    let bitrate: Handle<JsNumber> = cx.number(properties.audio_bitrate().unwrap_or(0));
    let sample_rate: Handle<JsNumber> = cx.number(properties.sample_rate().unwrap_or(0));
    let bit_depth: Handle<JsNumber> = cx.number(properties.bit_depth().unwrap_or(0));
    let duration_in_ms: Handle<JsNumber> = cx.number(duration as f64);
    let year: Handle<JsNumber> = cx.number(tag.year().unwrap_or(0));
    let track_number: Handle<JsNumber> = cx.number(tag.track().unwrap_or(0));
    let disc_number: Handle<JsNumber> = cx.number(tag.disk().unwrap_or(0));
    let lossless: Handle<JsBoolean> = cx.boolean(ident_lossless(file_ext.unwrap()));
    let container: Handle<JsString> = cx.string(file_ext.unwrap().to_ascii_uppercase());
    
    metadata_obj.set(&mut cx, "title", title)?;
    metadata_obj.set(&mut cx, "artist", artist)?;
    metadata_obj.set(&mut cx, "album", album)?;
    metadata_obj.set(&mut cx, "genre", genre)?;
    metadata_obj.set(&mut cx, "bitrate", bitrate)?;
    metadata_obj.set(&mut cx, "sample_rate", sample_rate)?;
    metadata_obj.set(&mut cx, "bit_depth", bit_depth)?;
    metadata_obj.set(&mut cx, "duration_in_ms", duration_in_ms)?;
    metadata_obj.set(&mut cx, "year", year)?;
    metadata_obj.set(&mut cx, "track_number", track_number)?;
    metadata_obj.set(&mut cx, "disc_number", disc_number)?;
    metadata_obj.set(&mut cx, "lossless", lossless)?;
    metadata_obj.set(&mut cx, "container", container)?;

    if tag.pictures().len() > 0 {
      let picture_b64: Handle<JsString> = cx.string(base64::encode(tag.pictures()[0].data()));
      metadata_obj.set(&mut cx, "artwork", picture_b64)?;
    }
    
    Ok(metadata_obj)
}

pub fn ident_lossless(file_ext: &str) -> bool { 
  let lowercase_ext: String = file_ext.to_ascii_lowercase();


  let result: bool = match lowercase_ext.as_str() {
    "wav" | "flac" => true,
    _ => false,
};

  return result
}