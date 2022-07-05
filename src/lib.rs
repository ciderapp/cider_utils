use neon::prelude::*;
use lofty::{Accessor, AudioFile, Probe, FileProperties};
use std::path::Path;
extern crate neon;
extern crate base64;


#[neon::main]
fn main(mut cx: ModuleContext) -> NeonResult<()> {
    cx.export_function("parseFile", parse_file)?;
    Ok(())
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
    let duration: f64 = properties.duration().as_secs_f64();
    //let picture: &[Picture] = tag.pictures();
    let metadata_obj: Handle<JsObject> = cx.empty_object();

    let title: Handle<JsString> = cx.string(tag.title().unwrap_or("0"));
    let artist: Handle<JsString> = cx.string(tag.artist().unwrap_or("0"));
    let album: Handle<JsString> = cx.string(tag.album().unwrap_or("0"));
    let genre: Handle<JsString> = cx.string(tag.genre().unwrap_or("0"));
    let bitrate: Handle<JsNumber> = cx.number(properties.audio_bitrate().unwrap_or(0));
    let sample_rate: Handle<JsNumber> = cx.number(properties.sample_rate().unwrap_or(0));
    let bit_depth: Handle<JsNumber> = cx.number(properties.bit_depth().unwrap_or(0));
    let duration_in_sec: Handle<JsNumber> = cx.number(duration);
    let year: Handle<JsNumber> = cx.number(tag.year().unwrap_or(0));
    let track_number: Handle<JsNumber> = cx.number(tag.track().unwrap_or(0));
    let disc_number: Handle<JsNumber> = cx.number(tag.disk().unwrap_or(0));
    metadata_obj.set(&mut cx, "title", title)?;
    metadata_obj.set(&mut cx, "artist", artist)?;
    metadata_obj.set(&mut cx, "album", album)?;
    metadata_obj.set(&mut cx, "genre", genre)?;
    metadata_obj.set(&mut cx, "bitrate", bitrate)?;
    metadata_obj.set(&mut cx, "sample_rate", sample_rate)?;
    metadata_obj.set(&mut cx, "bit_depth", bit_depth)?;
    metadata_obj.set(&mut cx, "duration_in_ms", duration_in_sec)?;
    metadata_obj.set(&mut cx, "year", year)?;
    metadata_obj.set(&mut cx, "track_number", track_number)?;
    metadata_obj.set(&mut cx, "disc_number", disc_number)?;
    if tag.pictures().len() > 0 {
      let picture_b64: Handle<JsString> = cx.string(base64::encode(tag.pictures()[0].data()));
      metadata_obj.set(&mut cx, "artwork", picture_b64)?;
    }
    
    Ok(metadata_obj)
}